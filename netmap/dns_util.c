#include <assert.h>
#include "dns_util.h"

static int verbose = 0;

#define MAC_ADDR_MAP_ITEMS 8
static macaddr_map_s g_macaddr_map[MAC_ADDR_MAP_ITEMS];
static int g_macaddr_map_inited = 0;

static char g_pkt_header[14 + 20 + 8] = {0};
static int g_pkt_header_inited = 0;
static unsigned short g_ip_id_next = 0x12a;

static int netmap_add_macaddr(char *smac, char *dmac, 
        unsigned int saddr, unsigned short source,
        unsigned short qid)
{
    int i = 0;

    if (g_macaddr_map_inited == 0)
    {
        g_macaddr_map_inited = 1;
        memset(g_macaddr_map, 0x0, MAC_ADDR_MAP_ITEMS * sizeof (macaddr_map_s));
    }

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
        printf("parse addr : saddr:%ld, daddr:%d, sport:%d, dport:%d, qid:%d\n",
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

    // change Ethnet header
    {
        unsigned char smac[ETH_ALEN]={0};
        unsigned char dmac[ETH_ALEN]={0};
        struct ethhdr *eh = (struct ethhdr *)buff;

        netmap_get_macmap(eh->h_source, eh->h_dest, 
                &ip->saddr, &udp->source, qid);
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
            goto NEXT_L; /* best effort! */
        }else {
            if (verbose > 1) D("echo: rx[%d] is DNS query", j);
            parse_addr(rxbuf, rxring->slot[j].len, iomsg);
            f = 1;
        }

        if (rs->buf_idx < 2) {
            sleep(2);
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

    while (ri < src->end) {
        rxring = NETMAP_RXRING(src->nifp, ri);

        if (rxring->avail == 0) {
            ri++;
            continue;
        }

        m += process_recv_ring(rxring, limit, iomsg);
        break;
    }

    return (m);
}

static int netmap_send_to_ring(struct my_ring *src, io_msg_s *iomsg)
{
    int limit = 512;
    struct netmap_ring *txring;
    u_int m = 0, ti=src->begin;

    while (ti < src->end) {
        txring = NETMAP_TXRING(src->nifp, ti);

        if (txring->avail == 0) {
            ti++;
            continue;
        }
        m += process_send_ring(txring, limit, iomsg);
        break;
    }

    return (m);
}

int netmap_recv(int fd, io_msg_s *iomsg ) 
{
    struct my_ring *ring;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    return netmap_recv_from_ring(ring, iomsg);
}


int netmap_send(int fd, io_msg_s *iomsg ) 
{
    struct my_ring *ring;

    ring = netmap_getring(fd);
    if (ring == NULL)
        return -1;

    return netmap_send_to_ring(ring, iomsg);
}



