/**
 * File: bind9_nm_io.c
 *
 * author: db
 *
 */
#include "nm_util.h"
#include "dns_util.h"
#include "bind9_nm_io.h"
#include "build.h"

static int verbose = -11;

static netmap_lock_t g_recv_lock;
static netmap_lock_t g_send_lock;

static pthread_mutex_t g_netmap_init_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

static char g_pkt_header[14 + 20 + 8] = {0};
static int g_pkt_header_inited = 0;
static unsigned short g_ip_id_next = 0x12a;

static int parse_addr(char *buff, int len, io_msg_s *iomsg)
{
    struct iphdr *ip;
    struct udphdr *udp;
    char *ip_buff = buff + 14;

    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    {
        iomsg->remote_addr = ip->saddr;
        iomsg->local_addr = ip->daddr;
        iomsg->remote_port = udp->source;
        iomsg->local_port = udp->dest;

        memcpy(iomsg->local_macaddr, buff, 6);
        memcpy(iomsg->remote_macaddr, buff + 6, 6);
    }

    if (verbose > -1)
    {
        char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
        unsigned short qid = ((unsigned short*) query)[0];
        printf("parse addr : remote addr:%ld, local addr:%d, remote port:%d, local port:%d, "
                " remote mac:-%x-%x-%x, local mac:-%x-%x-%x, "
                "qid:0x%x\n",
                (unsigned int )iomsg->remote_addr, 
                (unsigned int) iomsg->local_addr, 
                htons(iomsg->remote_port), htons(iomsg->local_port), 
                iomsg->remote_macaddr[3],
                iomsg->remote_macaddr[4],
                iomsg->remote_macaddr[5],
                iomsg->local_macaddr[3],
                iomsg->local_macaddr[4],
                iomsg->local_macaddr[5],
                qid);
    }

    // because under lock
    if (g_pkt_header_inited == 0)
    {
        g_pkt_header_inited = 1;
        memcpy(g_pkt_header, buff, 14 + 20 + 8);
    }

    return 0;
}

static int build_pkt_header(char *buff, int n, io_msg_s *iomsg)
{
    char check_buf[512] = {0};
    unsigned short qid;
    char *ip_buff = buff + 14;

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));

    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
    qid = ((unsigned short*) query)[0];

    // Filling Ethnet header
    {
        memcpy(buff, iomsg->remote_macaddr, 6);
        memcpy(buff + 6, iomsg->local_macaddr, 6);

        struct ethhdr *eh = (struct ethhdr *)buff;
        if (verbose > -1)
        {
            printf("build local mac:%x-%x-%x-%x-%x-%x," 
                    "remote mac:%x-%x-%x-%x-%x-%x,"
                    "local addr:%d,remote addr:%d, "
                    "local port:%d, remote port:%d, qid:0x%x\n",
                    eh->h_source[0],eh->h_source[1],eh->h_source[2],eh->h_source[3],eh->h_source[4],eh->h_source[5],
                    eh->h_dest[0],eh->h_dest[1],eh->h_dest[2],eh->h_dest[3],eh->h_dest[4],eh->h_dest[5],
                    (unsigned int )iomsg->local_addr,
                    (unsigned int )iomsg->remote_addr,
                    htons(iomsg->local_port), 
                    htons(iomsg->remote_port), 
                    qid);
        }
    }

    // Filling ip header
    ip->daddr = iomsg->remote_addr;
    ip->saddr = iomsg->local_addr;
    ip->tot_len = htons(n - 14);
    ip->id = (++g_ip_id_next & 0xFFFF);
    ip->check = 0;
    ip->check = in_cksum((unsigned short *)ip_buff, sizeof(struct iphdr));  

    // Filling UDP header
    udp->dest = iomsg->remote_port;
    udp->source = iomsg->local_port;
    udp->check = 0;

    // Calculate udp checksum
    {
        int udp_len = n - sizeof(struct iphdr ) - 14;
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
        u_int limit, io_msg_s *iomsg) 
{
    u_int j, m = 0;
    u_int f = 0;

    j = rxring->cur; /* RX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        assert( rs->len < iomsg->buff_len);

        if (0 != is_dns_query(rxbuf, rs->len)) {
            if (verbose > 1) D("rx[%d] is not DNS query", j);
            goto NEXT_L; /* best effort! */
        }else {
            if (verbose > 1) D("echo: rx[%d] is DNS query", j);
            parse_addr(rxbuf, rxring->slot[j].len, iomsg);
            f = 1;
        }

        /* copy the packet lenght. */
        if (rs->len < (14+20+8) || rs->len > 2048)
            D("wrong len %d rx[%d] ", rs->len, j);
        else if (verbose > 1)
            D("recv len %d rx[%d]", rs->len, j);

        memcpy(iomsg->buff, rxbuf + 14 + 20 + 8, rs->len - 14 - 20 - 8);
        iomsg->n = rs->len - 14 - 20 - 8;

NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (f == 1) break;
    }
    rxring->avail -= m;
    rxring->cur = j;

    return (m);
}

static int process_send_ring(struct netmap_ring *txring,
        u_int limit, io_msg_s *iomsg)
{
    u_int k, m = 0;

    k = txring->cur; /* TX */
    if (txring->avail < limit)
        limit = txring->avail;
    while (m < limit) {
        struct netmap_slot *ts = &txring->slot[k];
        char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

        ts->len = iomsg->buff_len + 14 + 20 + 8;
        memcpy(txbuf, g_pkt_header, 14 + 20 + 8);
        memcpy(txbuf + 14 + 20 + 8, iomsg->buff, iomsg->buff_len);

        build_pkt_header(txbuf, ts->len, iomsg);

        k = NETMAP_RING_NEXT(txring, k);
        ts->flags |= NS_BUF_CHANGED;
        m++;

        iomsg->n = iomsg->buff_len;
        break;
    }
    txring->avail -= m;
    txring->cur = k;

    return (m);
}

static int netmap_recv_from_ring(struct my_ring *src, io_msg_s *iomsg)
{
    int limit = 512;
    struct netmap_ring *rxring;
    u_int m = 0, ri = src->begin;

    while (ri < src->end) 
    {
        rxring = NETMAP_RXRING(src->nifp, ri);
        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

        m = process_recv_ring(rxring, limit, iomsg);
        break;
    }

    return (m);
}

static int netmap_send_to_ring(struct my_ring *src, io_msg_s *iomsg)
{
    int limit = 512;
    struct netmap_ring *txring;
    u_int m = 0, ti=src->begin;
    int times = 1;

LOOP_L:
    m = 0;
    ti=src->begin;
    while (ti < src->end) {
        txring = NETMAP_TXRING(src->nifp, ti);

        if (txring->avail == 0) 
        {
            ti++;
            continue;
        }   

        m = process_send_ring(txring, limit, iomsg);

        break;
    }

    if (m == 0 && times > 0)
    {
        int err = 0;
        struct nmreq req;
        bzero(&req, sizeof(req));
        req.nr_version = NETMAP_API;
        strncpy(req.nr_name, src->ifname, sizeof(req.nr_name));
        req.nr_ringid = 0;
        err = ioctl(src->fd, NIOCTXSYNC, &req);
        times --;
        
        {
            int delay = 100000;
            int busy = 0;
            while(--delay > 0) {busy ++;}
        }
        goto LOOP_L;
    }

    return (m);
}

int netmap_recv(int fd, io_msg_s *iomsg ) 
{
    struct my_ring *ring;
    int ret = 0;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    netmap_lock(&g_recv_lock);
    ret = netmap_recv_from_ring(ring, iomsg);
    netmap_unlock(&g_recv_lock);

    return ret;
}

int netmap_send(int fd, io_msg_s *iomsg) 
{
    struct my_ring *ring;
    int ret = 0;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    netmap_lock(&g_send_lock);
    ret = netmap_send_to_ring(ring, iomsg);
    netmap_unlock(&g_send_lock);

    return ret;
}

int netmap_init()
{
    if (pthread_mutex_trylock(&g_netmap_init_lock) == EBUSY)
        return 0;

    if (g_nm_inited == 1) return 0;
    g_nm_inited = 1;

#ifdef __BUILD_TIME 
    printf(" BUILD AT: %s\n", __BUILD_TIME);
#endif

#if defined(NM_USE_MUTEX_LOCK)
    printf(" NETMAP LOCK LIB: USE MUTEX LOCK\n ");
#elif defined(NM_USE_SPIN_LOCK)
    printf(" NETMAP LOCK LIB: USE SPIN LOCK\n ");
#else
    assert(0);
#endif

    netmap_lock_init(&g_recv_lock);
    netmap_lock_init(&g_send_lock);

    pthread_mutex_unlock(&g_netmap_init_lock);

    return 0;
}
