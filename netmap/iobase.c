/**
 * File: bind9_nm_io.c
 *
 * author: db
 *
 */
#include "iobase.h"

static int verbose = -11;

static netmap_lock_t g_recv_lock;
static netmap_lock_t g_send_lock;

static pthread_mutex_t g_netmap_init_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_nm_inited = 0;

static unsigned short g_ip_id_next = 0x12a;

static int parse_addr(char *buff, int len, io_block_t * block)
{
    struct iphdr *ip;
    struct udphdr *udp;
    char *ip_buff = buff + 14;

    ip = (struct iphdr*) (ip_buff);
    udp = (struct udphdr *) (ip_buff + sizeof(struct iphdr));

    {
        block->remote_addr = ip->saddr;
        block->local_addr = ip->daddr;
        block->remote_port = udp->source;
        block->local_port = udp->dest;

        memcpy(block->local_macaddr, buff, 6);
        memcpy(block->remote_macaddr, buff + 6, 6);
    }

    if (verbose > -1)
    {
        char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
        unsigned short qid = ((unsigned short*) query)[0];
        printf("parse addr : remote addr:%ld, local addr:%d, remote port:%d, local port:%d, "
                " remote mac:-%x-%x-%x, local mac:-%x-%x-%x, "
                "qid:0x%x\n",
                (unsigned int )block->remote_addr, 
                (unsigned int)block->local_addr, 
                htons(block->remote_port), htons(block->local_port), 
                block->remote_macaddr[3],
                block->remote_macaddr[4],
                block->remote_macaddr[5],
                block->local_macaddr[3],
                block->local_macaddr[4],
                block->local_macaddr[5],
                qid);
    }

    return 0;
}

static int build_pkt_header(char *buff, int len, io_block_t * block)
{
    char check_buf[IO_BLOCK_BUFF_SIZE] = {0};
    unsigned short qid = 0;
    char *ip_buff = buff + 14;

    struct iphdr* ip = (struct iphdr*)ip_buff; 
    struct udphdr * udp = (struct udphdr*) (ip_buff + sizeof(struct iphdr ));

    char *query = (char *)( ip_buff + sizeof(struct iphdr ) + sizeof(struct udphdr));
    qid = ((unsigned short*) query)[0];

    // Filling Ethnet header
    {
        memcpy(buff, block->remote_macaddr, 6);
        memcpy(buff + 6, block->local_macaddr, 6);

        // mark it is IP packet
        buff[12] = 0x8;
        buff[13] = 0x0;

        struct ethhdr *eh = (struct ethhdr *)buff;
        if (verbose > -1)
        {
            printf("build local mac:%x-%x-%x-%x-%x-%x," 
                    "remote mac:%x-%x-%x-%x-%x-%x,"
                    "local addr:%d,remote addr:%d, "
                    "local port:%d, remote port:%d, qid:0x%x\n",
                    eh->h_source[0],eh->h_source[1],eh->h_source[2],
                    eh->h_source[3],eh->h_source[4],eh->h_source[5],
                    eh->h_dest[0],eh->h_dest[1],eh->h_dest[2],
                    eh->h_dest[3],eh->h_dest[4],eh->h_dest[5],
                    (unsigned int )block->local_addr,
                    (unsigned int )block->remote_addr,
                    htons(block->local_port), 
                    htons(block->remote_port), 
                    qid);
        }
    }

    // Filling ip header
    ip->daddr = block->remote_addr;
    ip->saddr = block->local_addr;
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
    udp->dest = block->remote_port;
    udp->source = block->local_port;
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

static int process_recv_ring(struct netmap_ring *rxring, io_cache_t *cache) 
{
    u_int j, m = 0;
    u_int f = 0;
    io_block_t *block = NULL;
    u_int limit = IO_CACHE_BLOCKS_MAX; 

    j = rxring->cur; /* RX */
    if (rxring->avail < limit)
        limit = rxring->avail;
    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        if ( rs->len >= IO_BLOCK_BUFF_SIZE ||
                rs->len <= PROTO_LEN)
        {
            goto NEXT_L;
        }

        if (0 != is_dns_query(rxbuf, rs->len)) {
            goto NEXT_L; /* best effort! */
        }

        block = CACHE_GET_WRITEABLE_BLOCK(cache); 
        CACHE_FLUSH_W(cache);
        parse_addr(rxbuf, rs->len, block);

        memcpy(block->buffer, rxbuf + PROTO_LEN , rs->len - PROTO_LEN);
        block->data_len = rs->len - PROTO_LEN;

NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

        rs->flags |= NS_BUF_CHANGED;

        if (IS_CACHE_FULL(cache)) 
        {
            break;
        }
    }

    rxring->avail -= m;
    rxring->cur = j;

    return (m);
}

static int process_send_ring(struct netmap_ring *txring, io_cache_t *cache)
{
    u_int k, m = 0;
    u_int limit = cache->capcity; 
    io_block_t *block = NULL;

    k = txring->cur; /* TX */
    if (txring->avail < limit)
        limit = txring->avail;
    while (m < limit) {
        struct netmap_slot *ts = &txring->slot[k];
        char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

        block = CACHE_GET_READABLE_BLOCK(cache); 
        CACHE_FLUSH_R(cache);

        ts->len = block->data_len + PROTO_LEN; 
        memcpy(txbuf + PROTO_LEN, block->buffer, block->data_len);

        build_pkt_header(txbuf, ts->len, block);

        k = NETMAP_RING_NEXT(txring, k);
        ts->flags |= NS_BUF_CHANGED;
        m++;

        if (IS_CACHE_EMPTY(cache)) 
        {
            break;
        }
    }

    txring->avail -= m;
    txring->cur = k;

    return (m);
}

static int netmap_recv_from_ring(struct my_ring *src, io_cache_t *cache)
{
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

        m += process_recv_ring(rxring, cache);
        if (IS_CACHE_FULL(cache)) 
        {
            break;
        }

        ri ++;
    }

    return (m);
}

static int netmap_send_to_ring(struct my_ring *src, io_cache_t *cache)
{
    struct netmap_ring *txring;
    u_int m = 0, ti=src->begin;
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

        m += process_send_ring(txring, cache);
        if (IS_CACHE_EMPTY(cache)) 
        {
            break;
        }

        ti ++;
    }

    if (!IS_CACHE_EMPTY(cache) && times > 0)
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

int netmap_recv(int fd, io_cache_t *cache) 
{
    int ret = 0;
    struct my_ring *ring = NULL;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    netmap_lock(&g_recv_lock);

    if (IS_CACHE_FULL(cache))
        goto END_L;

    ret = netmap_recv_from_ring(ring, cache);

END_L:
    netmap_unlock(&g_recv_lock);

    return ret;
}

int netmap_send(int fd, io_cache_t *cache)
{
    int ret = 0;
    struct my_ring *ring = NULL;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    netmap_lock(&g_send_lock);

    if (IS_CACHE_EMPTY(cache))
        goto END_L;

    ret = netmap_send_to_ring(ring, cache);

END_L:
    netmap_unlock(&g_send_lock);

    return ret;
}

int netmap_send2(int fd, io_block_t *block)
{
    int ret = 0;
    struct my_ring *ring = NULL;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    netmap_lock(&g_send_lock);
    {
        io_cache_t cache;
        cache.writep = 1;
        cache.readp = 0;
        cache.capcity = 2;
        cache.blocks = block;
        ret = netmap_send_to_ring(ring, &cache);
    }
    netmap_unlock(&g_send_lock);

    return ret;
}

int netmap_init(void *arg)
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
