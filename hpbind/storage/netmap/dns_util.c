#include <assert.h>
#include <pthread.h>
#include "nm_util.h"
#include "dns_util.h"

static int verbose = 0;

#ifdef NM_DEBUG
#include <signal.h>
static int g_recv_pks_count = 0;
static int g_send_pks_count = 0;

static int g_recv_sig_count = 0;
static int g_send_sig_count = 0;

static int g_tx_ring_busy_count = 0;
static int g_rx_ring_busy_count = 0;
static int g_recv_non_dns_count = 0;

#define SIGNAL_NUM_PUT SIGUSR1 + 1 
void handle_signal(int no)
{
    if (no == SIGNAL_NUM_PUT)
    {
        printf("recv sig:%d\nsend sig:%d\n", g_recv_sig_count, g_send_sig_count);
        printf("recv pks:%d\nsend pks:%d\n", g_recv_pks_count, g_send_pks_count);
        printf("recv non dns pks:%d\n", g_recv_non_dns_count);
        printf("tx ring busy count:%d\n", g_tx_ring_busy_count);
        printf("rx ring busy count:%d\n", g_rx_ring_busy_count);

    }
}
#endif // NM_DEBUG

#if defined(NM_DBG_RECV_ECHO) || defined(NM_DBG_RECV_ECHO_SINGLE) || defined(NM_DBG_SEND_ECHO)
static int nm_debug_echo(struct my_ring *src);
static int dns_packet_process(char *buff, int len);
#endif

#ifdef NM_HAVE_G_LOCK
static netmap_lock_t g_recv_lock;
static netmap_lock_t g_send_lock;
#endif

static pthread_mutex_t g_netmap_init_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

#define MAC_ADDR_MAP_ITEMS 1024
static macaddr_map_s g_macaddr_map[MAC_ADDR_MAP_ITEMS];

static char g_pkt_header[14 + 20 + 8] = {0};
static int g_pkt_header_inited = 0;
static unsigned short g_ip_id_next = 0x12a;

static int netmap_add_macaddr(char *smac, char *dmac, 
        unsigned int saddr, unsigned short source,
        unsigned short qid)
{
    int i = 0;
    macaddr_map_s *m = &g_macaddr_map[i];

    for (i = 0; i < MAC_ADDR_MAP_ITEMS; i++ ) 
    {
        m = &g_macaddr_map[i];
        if (m->dirty == 0 ) 
        {
            memcpy(m->smac, smac, 6);
            memcpy(m->dmac, dmac, 6);
            m->saddr = saddr;
            m->source = source;
            m->qid = qid;
            m->dirty = 1;
            return 0;
        }
    }

    {
        memcpy(m->smac, smac, 6);
        memcpy(m->dmac, dmac, 6);
        m->saddr = saddr;
        m->source = source;
        m->qid = qid;
        m->dirty = 1;
    }

    return 0;
}

static int netmap_get_macmap(char *smac, char *dmac, 
        unsigned int *saddr,  unsigned short *source,
        unsigned short qid)
{
    int i = 0;
    macaddr_map_s *m = NULL;

    assert(smac != NULL);
    assert(dmac != NULL);
    assert(saddr != NULL);
    assert(source != NULL);

    for (i = 0; i < MAC_ADDR_MAP_ITEMS; i++ ) 
    {
        m = &g_macaddr_map[i];
        if (m->qid == qid ) 
        {
            memcpy(smac, m->smac, 6);
            memcpy(dmac, m->dmac, 6);
            *saddr = m->saddr;
            *source = m->source;
            m->dirty = 0;
            return 0;
        }
    }

    return -1;
}

struct pesudo_udphdr { 
    unsigned int saddr, daddr; 
    unsigned char unused; 
    unsigned char protocol; 
    unsigned short udplen; 
}; 

static unsigned short in_cksum(unsigned short *addr, int len) 
{ 
    int sum=0; 
    unsigned short res=0; 
    while( len > 1)  { 
        sum += *addr++; 
        len -=2; 
    } 
    if( len == 1) { 
        *((unsigned char *)(&res))=*((unsigned char *)addr); 
        sum += res; 
    } 
    sum = (sum >>16) + (sum & 0xffff); 
    sum += (sum >>16) ; 
    res = ~sum; 
    return res; 
}

static int is_dns_query(char *buff, int len)
{
    struct ethhdr *eh;
    struct iphdr *ip;
    struct udphdr *udp;

    char *ip_buff = buff + 14;

    eh = (struct ethhdr*)buff;
    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    if (eh->h_proto != ntohs(0x0800))
    {
        return 1;
    }

    if (ip->protocol != IPPROTO_UDP )
    {
        return 2;
    }

    if (udp->dest != ntohs(53))
    {
        return 3;
    }

    char *p = (ip_buff ) + 12;               
    if (verbose > 1)
    {
        printf("recv:%d, IP:%d.%d.%d.%d:%d => %d.%d.%d.%d:%d\n", len,
                p[0]&0XFF,p[1]&0XFF,p[2]&0XFF,p[3]&0XFF, htons(udp->source),
                p[4]&0XFF,p[5]&0XFF,p[6]&0XFF,p[7]&0XFF, htons(udp->dest)); 
    }

    return 0;
}

static int parse_addr(char *buff, int len, io_msg_s *iomsg)
{
    struct iphdr *ip;
    struct udphdr *udp;
    unsigned short qid;
    char *ip_buff = buff + 14;

    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));
    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));

    qid = ((unsigned short*) query)[0];

    iomsg->saddr = ip->saddr;
    iomsg->daddr = ip->daddr;
    iomsg->source = udp->source;
    iomsg->dest = udp->dest;

    if (verbose > 1)
    {
        printf("parse addr : saddr:%ld, daddr:%d, sport:%d, dport:%d, qid:0x%x\n",
                (unsigned int )iomsg->saddr, 
                (unsigned int) iomsg->daddr, 
                htons(iomsg->source), 
                htons(iomsg->dest), qid);
    }

    if (g_pkt_header_inited == 0)
    {
        g_pkt_header_inited = 1;
        memcpy(g_pkt_header, buff, 14 + 20 + 8);
    }

    netmap_add_macaddr(buff, buff + 6, ip->daddr, udp->dest, qid); 

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

#ifdef NM_DBG_SEND_ECHO
    //chage DNS query flag 
    query[2] |= 0x80;
#endif

    // change Ethnet header
    {
        unsigned char smac[ETH_ALEN]={0};
        unsigned char dmac[ETH_ALEN]={0};
        struct ethhdr *eh = (struct ethhdr *)buff;

        int ret = netmap_get_macmap(eh->h_source, eh->h_dest, 
                &ip->saddr, &udp->source, qid);

        if (ret != 0)
        {
            printf("ret(%d). build smac:%x-%x-%x-%x-%x-%x," 
                    "dmac:%x-%x-%x-%x-%x-%x,"
                    "ip->saddr:%d, sport:%d, qid:0x%x\n",
                    ret,
                    eh->h_source[0],eh->h_source[1],eh->h_source[2],eh->h_source[3],eh->h_source[4],eh->h_source[5],
                    eh->h_dest[0],eh->h_dest[1],eh->h_dest[2],eh->h_dest[3],eh->h_dest[4],eh->h_dest[5],
                    (unsigned int )ip->saddr,
                    htons(udp->source), qid);
        }

    }

    //Change ip header
    ip->daddr = iomsg->daddr;
    ip->tot_len = htons(n - 14);
    ip->id = (++g_ip_id_next & 0xFFFF);
    ip->check = 0;
    ip->check = in_cksum((unsigned short *)ip_buff, sizeof(struct iphdr));  

    // change UDP header
    udp->dest = iomsg->dest;
    udp->check = 0;

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
#ifdef NM_DEBUG
            g_recv_non_dns_count ++;
#endif
            goto NEXT_L; /* best effort! */
        }else {
            if (verbose > 1) D("echo: rx[%d] is DNS query", j);
            parse_addr(rxbuf, rxring->slot[j].len, iomsg);
            f = 1;

#ifdef NM_DEBUG
            g_recv_pks_count ++;
#endif
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

#ifdef NM_DBG_RECV_ECHO_SINGLE
static int process_recv_ring_single(struct netmap_ring *rxring, struct netmap_ring *txring,
        u_int limit, io_msg_s *iomsg) 
{
    u_int j, m = 0;
    u_int k =0, n = 0;
    u_int f = 0;

    j = rxring->cur; /* RX */
    k = txring->cur;
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        struct netmap_slot *ts = &txring->slot[k];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);
        unsigned int pkt;

        assert( rs->len < iomsg->buff_len);

        if (0 != is_dns_query(rxbuf, rs->len)) {
            if (verbose > 1) D("rx[%d] is not DNS query", j);
#ifdef NM_DEBUG
            g_recv_non_dns_count ++;
#endif
            goto NEXT_L; /* best effort! */
        }else {
            if (verbose > 1) D("echo: rx[%d] is DNS query", j);
            dns_packet_process(rxbuf, rxring->slot[j].len);
            
            f = 1;
#ifdef NM_DEBUG
            g_recv_pks_count ++;
#endif
        }

        /* copy the packet lenght. */
        if (rs->len < (14+20+8) || rs->len > 2048)
            D("wrong len %d rx[%d] ", rs->len, j);
        else if (verbose > 1)
            D("recv len %d rx[%d]", rs->len, j);

        pkt = ts->buf_idx;
        ts->buf_idx = rs->buf_idx;
        rs->buf_idx = pkt;
        ts->len = rs->len;

        /* report the buffer change. */
        ts->flags |= NS_BUF_CHANGED;
        k = NETMAP_RING_NEXT(txring, k);
        n ++;

        iomsg->n = rs->len - 14 - 20 - 8;
NEXT_L:	

        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (f == 1) break;
    }
    rxring->avail -= m;
    rxring->cur = j;
    txring->avail -= n;
    txring->cur = k;

    return (m);
}
#endif //  NM_DBG_RECV_ECHO_SINGLE

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

#ifdef NM_DEBUG
g_send_pks_count ++;
#endif
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

#ifdef NM_DBG_RECV_ECHO_SINGLE
    struct netmap_ring *txring;
    u_int ti = src->begin;
#endif
    
#ifdef NM_HAVE_RING_LOCK
    pthread_mutex_t *lock = NULL; 
#endif

#ifdef NM_DEBUG
    int flag = 0;
#endif

    while (ri < src->end 
#ifdef NM_DBG_RECV_ECHO_SINGLE
        && ti < src->end
#endif
            ) {
        rxring = NETMAP_RXRING(src->nifp, ri);
#ifdef NM_DBG_RECV_ECHO_SINGLE
        txring = NETMAP_TXRING(src->nifp, ti);
#endif

        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

#ifdef NM_DBG_RECV_ECHO_SINGLE
        if (txring->avail == 0) 
        {
            ti ++;
            continue;
        }
        m = process_recv_ring_single(rxring, txring, limit, iomsg);
#else
        m = process_recv_ring(rxring, limit, iomsg);
#endif

#ifdef NM_DEBUG
        flag = 1;
#endif

        break;
    }

#ifdef NM_DEBUG
    if (flag == 0)
        g_rx_ring_busy_count ++;
#endif

    return (m);
}

static int netmap_send_to_ring(struct my_ring *src, io_msg_s *iomsg)
{
    int limit = 512;
    struct netmap_ring *txring;
    u_int m = 0, times = 1, ti=src->begin;

#ifdef NM_DEBUG
    int flag = 0;
#endif

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

#ifdef NM_DEBUG 
        flag = 1;
#endif

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
        goto LOOP_L;
    }
#ifdef NM_DEBUG 
    if (flag == 0) 
        g_tx_ring_busy_count ++;
#endif

    return (m);
}

int netmap_recv(int fd, io_msg_s *iomsg ) 
{
    struct my_ring *ring;
    int ret = 0;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;
    
#if defined(NM_HAVE_G_LOCK)
    netmap_lock(&g_recv_lock);
#endif

#ifdef NM_DEBUG
    g_recv_sig_count ++;
#endif

#ifdef NM_DBG_RECV_ECHO
    (void) iomsg;
    ret = nm_debug_echo(ring);
#else
    ret = netmap_recv_from_ring(ring, iomsg);
#endif 

#if defined(NM_HAVE_G_LOCK)
    netmap_unlock(&g_recv_lock);
#endif

    return ret;
}

int netmap_send(int fd, io_msg_s *iomsg) 
{
    struct my_ring *ring;
    int ret = 0;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

#if defined(NM_HAVE_G_LOCK)
    netmap_lock(&g_send_lock);
#endif

#ifdef NM_DEBUG
    g_send_sig_count ++;
#endif

    ret = netmap_send_to_ring(ring, iomsg);

#if defined(NM_HAVE_G_LOCK)
     netmap_unlock(&g_send_lock);
#endif

    return ret;
}

int netmap_init()
{
    if (pthread_mutex_trylock(&g_netmap_init_lock) == EBUSY)
        return 0;
    
    if (g_nm_inited == 1) return 0;
    g_nm_inited = 1;
    
    printf(" BUILD AT: %s  %s\n", __DATE__, __TIME__);
#ifdef NM_DBG_RECV_ECHO
    printf(" NEMTAP DEBUG : AT RECV ECHO\n");
#elif defined(NM_DBG_RECV_ECHO_SINGLE)
    printf(" NEMTAP DEBUG : AT RECV ECHO SINGLE\n");
#elif defined(NM_DBG_SEND_ECHO)
    printf(" NEMTAP DEBUG : AT SEND ECHO\n");
#else
    printf(" NEMTAP DEBUG : NETMAP RECV + BIND QUERY + NETMAP SEND\n");
#endif
    
#ifdef NM_HAVE_G_LOCK
    printf(" NETMAP LOCK: HAVE GLOBAL LOCK\n"); 
#else
    printf(" NETMAP LOCK: NO ANY LOCK\n"); 
#endif

#if defined(NM_USE_MUTEX_LOCK)
    printf(" NETMAP LOCK LIB: USE MUTEX LOCK\n ");
#elif defined(NM_USE_SPIN_LOCK)
    printf(" NETMAP LOCK LIB: USE SPIN LOCK\n ");
#else
    assert(0);
#endif

#ifdef NM_DEBUG
    signal(SIGNAL_NUM_PUT, handle_signal);
    printf(" SIG NUM:%d\n", SIGNAL_NUM_PUT);
#endif
    memset(g_macaddr_map, 0x0, MAC_ADDR_MAP_ITEMS * sizeof (macaddr_map_s));
    
    netmap_lock_init(&g_recv_lock);
    netmap_lock_init(&g_send_lock);

    pthread_mutex_unlock(&g_netmap_init_lock);
    
    return 0;
}

#if  defined(NM_DBG_RECV_ECHO) || defined(NM_DBG_RECV_ECHO_SINGLE) || defined(NM_DBG_SEND_ECHO)

static int echo_dns_query(char *buff, int n)
{
    u_int32_t tmpaddr;
    u_int16_t tmpport;
    char check_buf[512] = {0};

    char *ip_buff = buff + 14;

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));
    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));

    //chage DNS query flag 
    query[2] |= 0x80;

    //Change ip header
    tmpaddr = ip->saddr;
    ip->saddr = ip->daddr;
    ip->daddr = tmpaddr;
    ip->check = 0;
    ip->check = in_cksum((unsigned short *)ip_buff, sizeof(struct iphdr));  

    // change UDP header
    tmpport = udp->source;
    udp->source = udp->dest;
    udp->dest = tmpport;
    udp->check = 0;

    {
        int udp_len = n - sizeof(struct iphdr ) - 14;

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

    // change Ethnet header
    {
        unsigned char mac_temp[ETH_ALEN]={0};
        struct ethhdr *eh = (struct ethhdr *)buff;
        
        memcpy(mac_temp,eh->h_dest,ETH_ALEN);
        memcpy(eh->h_dest, eh->h_source, ETH_ALEN);
        memcpy(eh->h_source, mac_temp, ETH_ALEN);
    }

    return 0;
}

static int dns_packet_process(char *buff, int len)
{
    assert(buff != NULL);
    assert(len > 42);

    echo_dns_query(buff, len);
    return 0;
}

static int process_rings(struct netmap_ring *rxring, 
        struct netmap_ring *txring,
        u_int limit)
{
    u_int j, k, m = 0;
    u_int f = 0;
    
    j = rxring->cur; /* RX */
    k = txring->cur; /* TX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    if (txring->avail < limit)
        limit = txring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        struct netmap_slot *ts = &txring->slot[k];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);
        uint32_t pkt;

        if (0 != is_dns_query(rxbuf, rs->len)) {
            //break; /* best effort! */
            goto NEXT_L;
        }else {
            dns_packet_process(rxbuf, rxring->slot[j].len);
        }

        pkt = ts->buf_idx;
        ts->buf_idx = rs->buf_idx;
        rs->buf_idx = pkt;
        ts->len = rs->len;

        /* report the buffer change. */
        ts->flags |= NS_BUF_CHANGED;
        k = NETMAP_RING_NEXT(txring, k);
        f ++;

NEXT_L:
        rs->flags |= NS_BUF_CHANGED;
        j = NETMAP_RING_NEXT(rxring, j);
        m++;
    }
    rxring->avail -= m;
    txring->avail -= f;
    rxring->cur = j;
    txring->cur = k;

    return (m);
}

/* move packts from src to destination */
static int nm_debug_echo(struct my_ring *src)
{
    struct netmap_ring *txring, *rxring;
    int limit = 1024;
    u_int m = 0, si = src->begin, di = src->begin;

    while (si < src->end && di < src->end) {
        rxring = NETMAP_RXRING(src->nifp, si);
        txring = NETMAP_TXRING(src->nifp, di);
       
        if (rxring->avail == 0) {
            si++;
            continue;
        }
        if (txring->avail == 0) {
            di++;
            continue;
        }
        m += process_rings(rxring, txring, limit);
        if (rxring->avail != 0 && txring->avail != 0)
            si++;
    }

    return (m);
}
#endif // NM_DBG_RECV_ECHO
