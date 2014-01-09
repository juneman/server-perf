/**
 * File: bind9_nm_io.c
 *
 * author: db
 *
 */
#include "iobase.h"

static int verbose = -11;

// for list netmap slot limited. 
static int g_limit = 512;

static pthread_mutex_t g_nm_inited_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

static unsigned short g_ip_id_next = 0x12a;

typedef struct __netmap_io_lock_t__ {
    netmap_lock_t recv_lock;
    netmap_lock_t send_lock;
}netmap_io_lock_t;

#define NM_IFNAME_SIZE (31)
#define NM_INTERFACES_NUM (8)
#define NM_FDS_NUM (32)
typedef struct __netmap_interfaces_info_t__ {
    struct my_ring ring;
    char ifname[NM_IFNAME_SIZE + 1];
    int fd;
    netmap_io_lock_t lock;
}netmap_interfaces_info_t;

static int g_interfaces_index = -1;
static netmap_interfaces_info_t g_interfaces_info[NM_INTERFACES_NUM];

#define MAX_FDS (4096)
static netmap_interfaces_info_t *g_fd_if_map[MAX_FDS];

static int netmap_init_interfaces_info() 
{
    memset(g_interfaces_info, 0x0, NM_INTERFACES_NUM * sizeof(netmap_interfaces_info_t));
    memset(g_fd_if_map, 0x0, MAX_FDS * sizeof(netmap_interfaces_info_t*));
    return 0;
}

// must by locked by g_netmap_fd_lock by caller
static netmap_interfaces_info_t * netmap_alloc_user_info_node(const char *ifname)
{
    int i = 0;
    netmap_interfaces_info_t * info = NULL; 

    for (i = 0; i < g_interfaces_index + 1; i ++)
    {
        info = &g_interfaces_info[i];
        if (strncmp(ifname, info->ifname, NM_IFNAME_SIZE) == 0)
        {
            return info;
        }
    }
    
    info = NULL;
    g_interfaces_index ++;
    if (g_interfaces_index >= NM_INTERFACES_NUM)
    {
        return NULL;
    }
    
    info = &g_interfaces_info[g_interfaces_index];
    if (info == NULL) 
        return NULL;
    info->ring.mem = NULL;
    info->fd = -1;

    netmap_lock_init(&(info->lock.send_lock));
    netmap_lock_init(&(info->lock.recv_lock));

    return info;
}

int netmap_openfd(const char *ifname)
{
    int fd = -1;
    netmap_interfaces_info_t *info = NULL;
    
    info = netmap_alloc_user_info_node(ifname);
    if (info == NULL)
    {
        return -1;
    }

    if (info->fd > 0)
    {
        fd = dup(info->fd);
        goto UPDATE_FDS;
    }

    strncpy(info->ifname, ifname, NM_IFNAME_SIZE);
    info->ring.ifname = info->ifname;
    netmap_open(&info->ring, 0, 0);
    
    if (info->ring.fd >= MAX_FDS)
    {
        return -1;
    }

    info->fd = info->ring.fd;
    fd = info->fd;

UPDATE_FDS:
    if (g_fd_if_map[fd] == NULL) 
    {
        g_fd_if_map[fd] = info;
    }

    return fd;
}

int netmap_closefd(int fd)
{
    netmap_interfaces_info_t *info = NULL;
    
    info = g_fd_if_map[fd];
    if (info == NULL)
    {
        return -1;
    }

    netmap_close(&(info->ring));
    g_fd_if_map[fd] = NULL;

    return 0;
}

static netmap_interfaces_info_t* netmap_get_interfaces_info(int fd)
{
    return g_fd_if_map[fd];
}

static int parse_addr(char *buff, int len, netmap_address_t *addr)
{
    struct iphdr *ip;
    struct udphdr *udp;
    char *ip_buff = buff + 14;

    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    {
        addr->remote_addr = ip->saddr;
        addr->local_addr = ip->daddr;
        addr->remote_port = udp->source;
        addr->local_port = udp->dest;

        memcpy(addr->local_macaddr, buff, 6);
        memcpy(addr->remote_macaddr, buff + 6, 6);
    }

    if (verbose > -1)
    {
        char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
        unsigned short qid = ((unsigned short*) query)[0];
        printf("parse addr : remote addr:%d, local addr:%d, remote port:%d, local port:%d, "
                " remote mac:-%x-%x-%x, local mac:-%x-%x-%x, "
                "qid:0x%x\n",
                (unsigned int )addr->remote_addr, 
                (unsigned int)addr->local_addr, 
                htons(addr->remote_port), htons(addr->local_port), 
                addr->remote_macaddr[3],
                addr->remote_macaddr[4],
                addr->remote_macaddr[5],
                addr->local_macaddr[3],
                addr->local_macaddr[4],
                addr->local_macaddr[5],
                qid);
    }

    return 0;
}

static int build_pkt_header(char *buff, int len, netmap_address_t *addr)
{
    char check_buf[NM_PKT_BUFF_SIZE_MAX] = {0};
    unsigned short qid = 0;
    char *ip_buff = buff + 14;

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));

    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
    qid = ((unsigned short*) query)[0];
    
    // Filling Ethnet header
    {
        memcpy(buff, addr->remote_macaddr, 6);
        memcpy(buff + 6, addr->local_macaddr, 6);

        // mark it is IP packet
        buff[12] = 0x8;
        buff[13] = 0x0;

        if (verbose > -1)
        {
            struct ethhdr *eh = (struct ethhdr *)buff;
            printf("build local mac:%x-%x-%x-%x-%x-%x," 
                    "remote mac:%x-%x-%x-%x-%x-%x,"
                    "local addr:%d,remote addr:%d, "
                    "local port:%d, remote port:%d, qid:0x%x\n",
                    eh->h_source[0],eh->h_source[1],eh->h_source[2],
                    eh->h_source[3],eh->h_source[4],eh->h_source[5],
                    eh->h_dest[0],eh->h_dest[1],eh->h_dest[2],
                    eh->h_dest[3],eh->h_dest[4],eh->h_dest[5],
                    (unsigned int )addr->local_addr,
                    (unsigned int )addr->remote_addr,
                    htons(addr->local_port), 
                    htons(addr->remote_port), 
                    qid);
        }
    }

    // Filling ip header
    ip->daddr = addr->remote_addr;
    ip->saddr = addr->local_addr;
    ip->tot_len = htons(len - 14);
    ip->id = (++g_ip_id_next & 0xFFFF);
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = 17;
    ip->check = 0;
    ip->check = in_cksum((unsigned short *)ip_buff, sizeof(struct iphdr));  

    // Filling UDP header
    udp->dest = addr->remote_port;
    udp->source = addr->local_port;
    udp->check = 0;

    // Calculate udp checksum
    {
        int udp_len = len - sizeof(struct iphdr ) - 14;
        udp->len = htons(udp_len);

        memset(check_buf, 0x0, 512);
        memcpy(check_buf + sizeof(struct pesudo_udphdr), (char*)udp, udp_len);
        struct pesudo_udphdr * pudph = (struct pesudo_udphdr *)check_buf;

        pudph->saddr = ip->saddr ; 
        pudph->daddr = ip->daddr; 
        pudph->unused=0; 
        pudph->protocol=IPPROTO_UDP; 
        pudph->udplen=htons(udp_len);

        udp->check = in_cksum((unsigned short *)check_buf, 
                udp_len +  sizeof(struct pesudo_udphdr) );
    }

    return 0;
}

static int process_recv_ring(struct netmap_ring *rxring, 
                char *buff, int *data_len, netmap_address_t *addr, int limit) 
{
    u_int j, m = 0;
    int recv_done = 0;

    j = rxring->cur; /* RX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        if ( rs->len >= NM_PKT_BUFF_SIZE_MAX ||
                rs->len <= PROTO_LEN)
        {
            goto NEXT_L;
        }

        if (0 != is_dns_query(rxbuf, rs->len)) 
        {
           goto NEXT_L; /* best effort! */
        }
        
        if (addr != NULL)
        {
            parse_addr(rxbuf, rs->len, addr);
        }

        memcpy(buff, rxbuf + PROTO_LEN , rs->len - PROTO_LEN);
        *data_len = rs->len - PROTO_LEN;

        recv_done = 1;
NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (recv_done == 1) 
        {
            break;
        }
    }

    rxring->avail -= m;
    rxring->cur = j;
    
    if (recv_done == 0) 
        return NM_FAILED;

    return (NM_SUCCESS);
}

static int process_send_ring(struct netmap_ring *txring, 
                char *buff, int data_len, netmap_address_t *addr)
{
    u_int k = txring->cur; /* TX */
    struct netmap_slot *ts = &txring->slot[k];
    char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

    ts->len = data_len + PROTO_LEN; 
    memcpy(txbuf + PROTO_LEN, buff, data_len);

    build_pkt_header(txbuf, ts->len, addr);

    k = NETMAP_RING_NEXT(txring, k);
    ts->flags |= NS_BUF_CHANGED;

    txring->avail -= 1;
    txring->cur = k;

    return (NM_SUCCESS);
}

static int netmap_recv_from_ring(struct my_ring *src, 
                char *buff, int *data_len, netmap_address_t *addr, int limit)
{
    struct netmap_ring *rxring;
    u_int ri = src->begin;
    int ret = NM_FAILED;

    while (ri < src->end) 
    {
        rxring = NETMAP_RXRING(src->nifp, ri);
        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

        ret = process_recv_ring(rxring, buff, data_len, addr, limit);
        if (ret == NM_SUCCESS) 
        {
            break;
        }

        ri ++;
    }

    return ret;
}

static int netmap_send_to_ring(struct my_ring *src, 
            char *buff, int data_len, netmap_address_t *addr)
{
    struct netmap_ring *txring;
    u_int ret = 0, ti=src->begin;
    int times = 1;

LOOP_L:
    ti=src->begin;
    while (ti < src->end) {
        txring = NETMAP_TXRING(src->nifp, ti);

        if (txring->avail == 0) 
        {
            ti++;
            continue;
        }   

        ret = process_send_ring(txring, buff, data_len, addr);
        if (ret == NM_SUCCESS) 
        {
            return ret;
        }

        ti ++;
    }

    if (ret != NM_SUCCESS && times > 0)
    {
        int err = 0;
        struct nmreq req;
        bzero(&req, sizeof(req));
        req.nr_version = NETMAP_API;
        strncpy(req.nr_name, src->ifname, sizeof(req.nr_name));
        req.nr_ringid = 0;
        err = ioctl(src->fd, NIOCTXSYNC, &req);
        times --;
        goto LOOP_L;
    }

    return NM_FAILED;
}

int netmap_recv(int fd, char *buff, int *data_len, netmap_address_t *addr) 
{
    int ret = NM_FAILED;
    netmap_interfaces_info_t *info = NULL;

    if (buff == NULL || data_len == NULL)
    {
        return NM_ARGS_NULL;
    }

    info = netmap_get_interfaces_info(fd);
    if (info == NULL)
    {
        return NM_MEM_ERROR;
    }

    netmap_lock(&(info->lock.recv_lock));
    ret = netmap_recv_from_ring(&(info->ring), buff, data_len, addr, g_limit);
    netmap_unlock(&(info->lock.recv_lock));

    return ret;
}

int netmap_send(int fd, char *buff, int data_len, netmap_address_t *addr)
{
    int ret = NM_FAILED;
    netmap_interfaces_info_t *info = NULL;

    if (buff == NULL || data_len <= 0 || addr == NULL)
    {
        return NM_ARGS_NULL;
    }

    info = netmap_get_interfaces_info(fd);
    if (info == NULL)
    {
        return NM_MEM_ERROR;
    }

    netmap_lock(&(info->lock.send_lock));
    ret = netmap_send_to_ring(&(info->ring), buff, data_len, addr);
    netmap_unlock(&(info->lock.send_lock));

    return ret;
}

// only in main thread call ONCE time.
int netmap_init(void)
{
    if (pthread_mutex_trylock(&g_nm_inited_lock) == EBUSY)
    {
        return NM_SUCCESS;
    }

    if (g_nm_inited == 1)
    {
        return NM_SUCCESS;
    }

    g_nm_inited = 1;

#ifdef __BUILD_TIME 
    printf(" BUILD AT: %s\n", __BUILD_TIME);
#endif

#if defined(NM_USE_MUTEX_LOCK)
    D(" NETMAP LOCK LIB: USE MUTEX LOCK\n ");
#elif defined(NM_USE_SPIN_LOCK)
    D(" NETMAP LOCK LIB: USE SPIN LOCK\n ");
#else
    assert(0);
#endif

    netmap_init_interfaces_info();

    pthread_mutex_unlock(&g_nm_inited_lock);

    return NM_SUCCESS;
}

int netmap_destroy(void)
{
    // do cleanup work
    return NM_SUCCESS;
}
