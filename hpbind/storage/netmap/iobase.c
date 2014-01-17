/**
 * File: bind9_nm_io.c
 *
 * author: db
 *
 */
#include <unistd.h>
#include <sys/epoll.h>
#include "iobase.h"
#include "squeue.h"

static int verbose = -11;

// for list netmap slot limited. 
static int g_limit = 512;

static pthread_mutex_t g_nm_inited_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

static unsigned short g_ip_id_next = 0x11;

typedef struct __netmap_io_lock_t__ {
    slock_t recv_lock;
    slock_t send_lock;
    slock_t mgtlock;
}netmap_io_lock_t;

#define NM_IFNAME_SIZE (31)
typedef struct __netmap_interfaces_info_t__ {
    struct my_ring ring;
    char ifname[NM_IFNAME_SIZE + 1];
    int pipe_fds[2];
    int nmfd;
    netmap_io_lock_t lock;
}netmap_interfaces_info_t;

#define MAX_FDS (512)
typedef struct __io_manager_t__ 
{
    int index;
    int stopped;
    pthread_t run_pid;
    netmap_interfaces_info_t interfaces_info[MAX_FDS];
    netmap_interfaces_info_t *fd_if_map[MAX_FDS];
    
    squeue_t recv_queue[MAX_FDS];
    squeue_t send_queue[MAX_FDS];
    squeue_t idle_queue;

}io_manager_t;

static io_manager_t g_io_mgr;

static void netmap_init_interfaces_info() 
{
    memset(&g_io_mgr, 0x0, sizeof(io_manager_t));
    g_io_mgr.index = -1;
    
    int index = 0;
    for (index = 0; index < MAX_FDS; index ++ )
    {
        squeue_init(&(g_io_mgr.recv_queue[index]));
        squeue_init(&(g_io_mgr.send_queue[index]));
    }
    
    squeue_prealloc(&(g_io_mgr.idle_queue), 2048);
}

// must by locked by g_netmap_fd_lock by caller
static netmap_interfaces_info_t * netmap_get_available_if_info_node(const char *ifname)
{
    netmap_interfaces_info_t * info = NULL; 

    g_io_mgr.index ++;
    if (g_io_mgr.index >= MAX_FDS)
    {
        return NULL;
    }

    info = &(g_io_mgr.interfaces_info[g_io_mgr.index]);
    info->ring.mem = NULL;
   
    slock_init(&(info->lock.send_lock));
    slock_init(&(info->lock.recv_lock));
    slock_init(&(info->lock.mgtlock));

    return info;
}

static netmap_interfaces_info_t* netmap_get_interfaces_info(int fd)
{
    if (fd < MAX_FDS)
    {
        return g_io_mgr.fd_if_map[fd];
    }

    D("err: fd > %d", MAX_FDS);
    return NULL;
}

static void netmap_set_interfaces_info(int fd, netmap_interfaces_info_t * info)
{
    if (fd < MAX_FDS)
    {
        g_io_mgr.fd_if_map[fd] = info;
    }
    else
    {
        D("err: fd > %d", MAX_FDS);
    }
}

int netmap_openfd(const char *ifname)
{
    int fd = -1, err = -1;
    netmap_interfaces_info_t *info = NULL;

    info = netmap_get_available_if_info_node(ifname);
    if (info == NULL)
    {
        return NM_R_FAILED;
    }

    slock_lock(&(info->lock.mgtlock));
     
    strncpy(info->ifname, ifname, NM_IFNAME_SIZE);
    info->ring.ifname = info->ifname;
    err = netmap_open(&info->ring, 0, 0);

    if (err != NM_R_SUCCESS)
    {
        goto END_L;
    }

    if (info->ring.fd >= MAX_FDS)
    {
        netmap_close(&(info->ring));
        close(info->ring.fd);
        goto END_L;
    }
 
    if (pipe(info->pipe_fds) < 0 )
    {
        D("pipe failed.");
        goto END_L;
    } else {
        int flags = fcntl(info->pipe_fds[0], F_GETFL, 0); 
        fcntl( info->pipe_fds[0], F_SETFL, flags | O_NONBLOCK);

        flags = fcntl(info->pipe_fds[1], F_GETFL, 0); 
        fcntl( info->pipe_fds[1], F_SETFL, flags | O_NONBLOCK);
    }

    info->nmfd = info->ring.fd;
    netmap_set_interfaces_info(info->nmfd, info);
    fd = info->pipe_fds[0];
    netmap_set_interfaces_info(fd, info);

END_L:
    slock_unlock(&(info->lock.mgtlock));

    return fd;
}

int netmap_closefd(int fd)
{
    netmap_interfaces_info_t *info = NULL;

    info = netmap_get_interfaces_info(fd);
    if (info == NULL)
    {
        return NM_R_FAILED;
    }

    slock_lock(&(info->lock.mgtlock));

    slock_destroy(&(info->lock.send_lock));
    slock_destroy(&(info->lock.recv_lock));
    netmap_close(&(info->ring));
    
    close(info->pipe_fds[0]);
    close(info->pipe_fds[1]);
    close(info->nmfd);

    netmap_set_interfaces_info(fd, NULL);
    netmap_set_interfaces_info(info->nmfd, NULL);

    slock_unlock(&(info->lock.mgtlock));

    return NM_R_SUCCESS;
}

static int parse_addr(const char *buff, int len, netmap_address_t *addr)
{
    struct ethhdr *eh = NULL; 
    struct iphdr *ip = NULL; 
    struct udphdr *udp = NULL; 
    char *ip_buff = NULL;

    eh = (struct ethhdr *)buff;
    ip_buff =(char*) (((char*)eh) + sizeof(struct ethhdr));
    
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

    return NM_R_SUCCESS;
}

static int build_pkt_header(char *buff, int len, const netmap_address_t *addr)
{
    char *ip_buff = buff + sizeof(struct ethhdr);

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));

    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));

    // Filling Ethnet header
    memcpy(buff, addr->remote_macaddr, 6);
    memcpy(buff + 6, addr->local_macaddr, 6);

    // mark it is IP packet
    buff[12] = 0x8;
    buff[13] = 0x0;

    if (verbose > -1)
    {
        struct ethhdr *eh = (struct ethhdr *)buff;
        unsigned short qid = ((unsigned short*) query)[0];
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
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->check = wrapsum(checksum(ip, sizeof(struct iphdr), 0)); 

    // Filling UDP header
    udp->len = htons(len - sizeof(struct iphdr ) - sizeof(struct ethhdr));
    udp->dest = addr->remote_port;
    udp->source = addr->local_port;
    udp->check = 0;
    udp->check = wrapsum(checksum(udp, sizeof(struct udphdr),
                checksum(query,
                    len - PROTO_LEN,
                    checksum(&ip->saddr, 2 * sizeof(ip->saddr),
                        IPPROTO_UDP + (u_int32_t)ntohs(udp->len)
                        )
                    )
                ));

    return NM_R_SUCCESS;
}

static int process_recv_ring(struct netmap_ring *rxring, 
        char *buff, int buff_len, netmap_address_t *addr, int limit) 
{
    u_int j, m = 0;
    int recv_bytes = 0;

    j = rxring->cur; /* RX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        if ( rs->len >= NM_PKT_BUFF_SIZE_MAX
                || rs->len >= buff_len + PROTO_LEN
                || rs->len <= PROTO_LEN)
        {
            goto NEXT_L;
        }

        if (NM_TRUE != is_dns_query(rxbuf, rs->len)) 
        {
            goto NEXT_L; /* best effort! */
        }

        if (addr != NULL)
        {
            parse_addr(rxbuf, rs->len, addr);
        }

        recv_bytes = rs->len - PROTO_LEN;
        memcpy(buff, rxbuf + PROTO_LEN , recv_bytes);

NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (recv_bytes > 0) 
        {
            break;
        }
    }

    rxring->avail -= m;
    rxring->cur = j;

    if (recv_bytes <= 0) 
    {
        return NM_R_FAILED;
    }

    return (recv_bytes);
}

static int process_send_ring(struct netmap_ring *txring, 
        const char *buff, int data_len, const netmap_address_t *addr)
{
    u_int k = txring->cur; /* TX */
    struct netmap_slot *ts = &txring->slot[k];
    char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

    assert(data_len > 0);

    ts->len = data_len + PROTO_LEN; 
    memcpy(txbuf + PROTO_LEN, buff, data_len);

    build_pkt_header(txbuf, ts->len, addr);

    k = NETMAP_RING_NEXT(txring, k);
    ts->flags |= NS_BUF_CHANGED;

    txring->avail -= 1;
    txring->cur = k;

    return (data_len);
}

static int netmap_recv_from_ring(struct my_ring *src, 
        char *buff, int buff_len, netmap_address_t *addr, int limit)
{
    struct netmap_ring *rxring;
    u_int ri = src->begin;
    int ret = NM_R_FAILED;

    while (ri < src->end) 
    {
        rxring = NETMAP_RXRING(src->nifp, ri);
        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

        ret = process_recv_ring(rxring, buff, buff_len, addr, limit);
        if (ret > 0) 
        {
            break;
        }

        ri ++;
    }

    return ret;
}

static int netmap_send_to_ring(struct my_ring *src, 
        const char *buff, int data_len, const netmap_address_t *addr)
{
    struct netmap_ring *txring;
    u_int ti=src->begin;
    int times = 1;
    int ret = NM_R_FAILED;

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
        if (ret > 0) 
        {
            return ret;
        }

        ti ++;
    }

    if (ret <= 0 && times > 0)
    {
        struct nmreq req;
        bzero(&req, sizeof(req));
        req.nr_version = NETMAP_API;
        strncpy(req.nr_name, src->ifname, sizeof(req.nr_name));
        req.nr_ringid = 0;
        ioctl(src->fd, NIOCTXSYNC, &req);
        times --;
        goto LOOP_L;
    }

    return NM_R_FAILED;
}

int netmap_recv(int fd, char *buff, int buff_len, netmap_address_t *addr) 
{
    char buf[1];
    int ret = NM_R_FAILED;
    netmap_interfaces_info_t *info = NULL;

    if (buff == NULL || buff_len <= 0)
    {
        return NM_R_ARGS_NULL;
    }

    info = netmap_get_interfaces_info(fd);
    if (info == NULL)
    {
        return NM_R_FD_OUTOFBIND;
    }
    read(info->pipe_fds[0], buf, sizeof(buf));
   
    sqmsg_t *msg = squeue_pop(&(g_io_mgr.recv_queue[info->nmfd]));
    if (NULL == msg) return ret;
    
    if (buff_len < msg->len) 
    {
        ret = NM_R_BUFF_NOT_ENOUGH;
        goto END_L;
    }

    memcpy(buff, msg->buff, msg->len);
    
    if (addr != NULL)
        memcpy(addr, msg->extbuff, sizeof(netmap_address_t));
    ret = msg->len;

END_L:
    squeue_push(&(g_io_mgr.idle_queue), msg);

    return ret;
}

int netmap_send(int fd, const char *buff, int data_len, const netmap_address_t *addr)
{
    int ret = NM_R_FAILED;
    netmap_interfaces_info_t *info = NULL;
    
    assert(buff != NULL);
    assert(data_len > 0 && data_len < NM_PKT_BUFF_SIZE_MAX);
    assert(addr != NULL);

    if (buff == NULL 
            || data_len <= 0 
            || data_len >= NM_PKT_BUFF_SIZE_MAX
            || addr == NULL )
    {
        return NM_R_ARGS_NULL;
    }

    info = netmap_get_interfaces_info(fd);
    if (info == NULL)
    {
        return NM_R_FD_OUTOFBIND;
    }

    sqmsg_t *msg = squeue_pop(&(g_io_mgr.idle_queue));
    if (NULL == msg) return ret;
    
    memcpy(msg->buff, buff, data_len);
    memcpy(msg->extbuff, addr, sizeof(netmap_address_t));
    msg->len = data_len;
    
    squeue_push(&(g_io_mgr.send_queue[info->nmfd]), msg);

    return ret;
}

// only in main thread call ONCE time.
int netmap_init(void)
{
    if (pthread_mutex_trylock(&g_nm_inited_lock) == EBUSY)
    {
        return NM_R_SUCCESS; 
    }

    if (g_nm_inited == 1)
    {
        return NM_R_SUCCESS; 
    }

    g_nm_inited = 1;

#ifdef __BUILD_TIME 
    printf(" BUILD AT: %s\n", __BUILD_TIME);
#endif

    netmap_init_interfaces_info();

    pthread_mutex_unlock(&g_nm_inited_lock);

    return NM_R_SUCCESS; 
}

static int __netmap_recvfrom__(int fd)
{
    sqmsg_t *msg = NULL; 
    int have_data = NM_R_FAILED;
    netmap_interfaces_info_t* info = NULL;
    
    info = netmap_get_interfaces_info(fd);
    if (NULL == info) return have_data;
    
    
    squeue_t *idle_queue = &(g_io_mgr.idle_queue);
    msg = squeue_pop(idle_queue); 
    if (NULL != msg) 
    {
        squeue_t *recv_queue = &(g_io_mgr.recv_queue[info->nmfd]);
        netmap_address_t *addr = (netmap_address_t *)(&msg->extbuff);
        int recv_bytes = netmap_recv_from_ring(&(info->ring), 
                                            msg->buff, SQMSG_SIZE_MAX, addr, g_limit);
        if (recv_bytes > 0) 
        { 
            msg->len = recv_bytes;
            squeue_push(recv_queue, msg);
            have_data = NM_R_SUCCESS;
        }
        else
        {
            squeue_push(idle_queue, msg);
        }
    }

    return have_data;
}

static int __netmap_sendto__(int fd)
{
    sqmsg_t *msg = NULL; 
    netmap_interfaces_info_t* info = NULL;
    
    info = netmap_get_interfaces_info(fd);
    if (NULL == info) return NM_R_FAILED;

    squeue_t *send_queue = &(g_io_mgr.send_queue[info->nmfd]);
    msg = squeue_pop(send_queue);
    if (NULL != msg) 
    {
        squeue_t *idle_queue = &(g_io_mgr.idle_queue);
        netmap_send_to_ring(&(info->ring), msg->buff, msg->len, 
                                        (netmap_address_t*)(msg->extbuff));
        squeue_push(idle_queue, msg);
    }

    return NM_R_SUCCESS;
}

static int __netmap_signal__(int fd)
{
    char buf[1] = {'a'};
    netmap_interfaces_info_t* info = NULL;
    
    info = netmap_get_interfaces_info(fd);
    if (NULL != info)
    {
        write(info->pipe_fds[1], buf, sizeof(buf));
        return NM_R_SUCCESS;
    }

    return NM_R_FAILED;
}


void * __netmap_run__(void*args)
{
	struct epoll_event event, *events;
	int nfd, efd, s;
    int index = 0;
    int ecount = g_io_mgr.index + 1; 

    //----------------------------
    assert(ecount > 0);
	efd = epoll_create(ecount);
	if (efd == -1)
	{
		D("epoll_create failed");
        exit(1);
	} 
    
    int nmfd = -1;
    for (index = 0; index < ecount; index ++)
    {
        nmfd = g_io_mgr.interfaces_info[index].nmfd;
        event.data.fd = nmfd; 
        event.events = EPOLLIN | EPOLLOUT;
        s = epoll_ctl(efd, EPOLL_CTL_ADD, nmfd, &event);
        if (s == -1) {
            D("epoll_ctl error");
            exit(1);
        }
    }

	events = calloc(ecount, sizeof(struct epoll_event));
    if (events == NULL) 
    {
        D("calloc failed");
        exit(1);
    }

    while (!g_io_mgr.stopped) 
    {
        nfd = epoll_wait(efd, events, ecount, -1);
        if (nfd < 0) continue;
               for (index = 0; index < nfd; index ++)
        {
            int ppfd = events[index].data.fd;
            if ( events[index].events & EPOLLIN 
                    || events[index].events & EPOLLOUT)
            {
                int have_data =  __netmap_recvfrom__(ppfd);
                if (NM_R_SUCCESS == have_data) __netmap_signal__(ppfd);
                __netmap_sendto__(ppfd);
            }
        }
    }

    D("exiting");
   	return NULL;
}

void netmap_setup(void)
{
    pthread_create(&(g_io_mgr.run_pid), NULL, __netmap_run__, NULL);
    pthread_detach(g_io_mgr.run_pid);
}

void netmap_teardown(void)
{
    g_io_mgr.stopped = NM_TRUE;
}

int netmap_destroy(void)
{
   
    return NM_R_SUCCESS; 
}





