/**
 * File: netmapio.c
 *
 * author: db
 *
 */
#include "types.h"
#include "netmapio.h"
static int verbose = 0;

static int netmap_do_ioctl(netmapio_ringobj *me, u_long what, int subcmd)
{
    struct ifreq ifr;
    int error;

    struct ethtool_value eval;
    int fd; 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("Error: cannot get device control socket.\n");
        return NM_R_FAILED;
    }

    (void)subcmd;	// unused
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, me->ifname, sizeof(ifr.ifr_name));
    switch (what) {
        case SIOCSIFFLAGS:
            ifr.ifr_flagshigh = me->if_flags >> 16;
            ifr.ifr_flags = me->if_flags & 0xffff;
            break;

        case SIOCETHTOOL:
            eval.cmd = subcmd;
            eval.data = 0;
            ifr.ifr_data = (caddr_t)&eval;
            break;
    }
    error = ioctl(fd, what, &ifr);
    if (error)
        goto done;
    switch (what) {
        case SIOCGIFFLAGS:
            me->if_flags = (ifr.ifr_flagshigh << 16) |
                (0xffff & ifr.ifr_flags);
            if (verbose)
                printf("flags are 0x%x\n", me->if_flags);
            break;
    }
done:
    close(fd);
    if (error)
    {
        printf("ioctl error %d %lu\n", error, what);
        return NM_R_FAILED;
    }
    return NM_R_SUCCESS;
}

/*
 * open a device. if me->mem is null then do an mmap.
 * Returns the file descriptor.
 * The extra flag checks configures promisc mode.
 */
int netmap_open(netmapio_ringobj *me, int ringid, int promisc)
{
    int fd, err, l;
    struct nmreq req;

    me->fd = fd = open("/dev/netmap", O_RDWR);
    if (fd < 0) {
        printf("Unable to open /dev/netmap\n");
        return NM_R_FAILED;
    }
    bzero(&req, sizeof(req));
    req.nr_version = NETMAP_API;
    strncpy(req.nr_name, me->ifname, sizeof(req.nr_name));
    req.nr_ringid = ringid;
    err = ioctl(fd, NIOCGINFO, &req);
    if (err) {
        printf("cannot get info on %s, errno %d ver %d\n",
                me->ifname, errno, req.nr_version);
        goto error;
    }
    me->memsize = l = req.nr_memsize;
    if (verbose)
        printf("memsize is %d MB\n", l>>20);
    err = ioctl(fd, NIOCREGIF, &req);
    if (err) {
        printf("Unable to register %s\n", me->ifname);
        goto error;
    }

    if (me->mem == NULL) {
        me->mem = mmap(0, l, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
        if (me->mem == MAP_FAILED) {
            printf("Unable to mmap\n");
            me->mem = NULL;
            goto error;
        }
    }

    /* Set the operating mode. */
    if (ringid != NETMAP_SW_RING) {
        netmap_do_ioctl(me, SIOCGIFFLAGS, 0);
        if ((me[0].if_flags & IFF_UP) == 0) {
            printf("%s is down, bringing up...\n", me[0].ifname);
            me[0].if_flags |= IFF_UP;
        }
        if (promisc) {
            me[0].if_flags |= IFF_PPROMISC;
            netmap_do_ioctl(me, SIOCSIFFLAGS, 0);
        }
    }

    me->nifp = NETMAP_IF(me->mem, req.nr_offset);
    me->queueid = ringid;
    if (ringid & NETMAP_SW_RING) {
        me->begin = req.nr_rx_rings;
        me->end = me->begin + 1;
        me->tx = NETMAP_TXRING(me->nifp, req.nr_tx_rings);
        me->rx = NETMAP_RXRING(me->nifp, req.nr_rx_rings);
    } else if (ringid & NETMAP_HW_RING) {
        printf("XXX check multiple threads\n");
        me->begin = ringid & NETMAP_RING_MASK;
        me->end = me->begin + 1;
        me->tx = NETMAP_TXRING(me->nifp, me->begin);
        me->rx = NETMAP_RXRING(me->nifp, me->begin);
    } else {
        me->begin = 0;
        me->end = req.nr_rx_rings; // XXX max of the two
        me->tx = NETMAP_TXRING(me->nifp, 0);
        me->rx = NETMAP_RXRING(me->nifp, 0);
    }
    return NM_R_SUCCESS;
error:
    close(me->fd);
    return NM_R_FAILED;
}

int netmap_close(netmapio_ringobj *me)
{
    if (me->mem)
        munmap(me->mem, me->memsize);
    return NM_R_SUCCESS;
}

void netmap_tx_force(const netmapio_ringobj *obj)
{
    struct nmreq req;
    bzero(&req, sizeof(req));
    req.nr_version = NETMAP_API;
    strncpy(req.nr_name, obj->ifname, sizeof(req.nr_name));
    req.nr_ringid = obj->queueid;
    ioctl(obj->fd, NIOCTXSYNC, &req);
}

static int process_recv_from_ring(struct netmap_ring *rxring, 
        NM_BOOL (*filter)(const char *buff, int len), 
        int (*parse_header)(const char *buff, int len, netmap_address_t *addr),
        char *buff, int buff_len, netmap_address_t *addr) 
{
    assert(rxring != NULL);
    assert(filter != NULL);
    assert(parse_header != NULL);
    assert(buff != NULL);
    assert(buff_len > 0);
    assert(addr != NULL);

    u_int j, m = 0;
    int recv_bytes = 0;
    int limit = rxring->avail;
    j = rxring->cur; /* RX */

    while (m < limit) {
        struct netmap_slot *rs = &rxring->slot[j];
        char *rxbuf = NETMAP_BUF(rxring, rs->buf_idx);

        if ( rs->len >= NM_PKT_BUFF_SIZE_MAX
                || rs->len >= buff_len + PROTO_LEN
                || rs->len <= PROTO_LEN)
        {
            goto NEXT_L;
        }

        if (NM_TRUE != filter(rxbuf, rs->len))
        {
            goto NEXT_L;
        }

        if (addr != NULL) parse_header(rxbuf, rs->len, addr);

        recv_bytes = rs->len - PROTO_LEN;
        memcpy(buff, rxbuf + PROTO_LEN , recv_bytes);

NEXT_L:	
        j = NETMAP_RING_NEXT(rxring, j);
        m++;

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

static int process_send_to_ring(struct netmap_ring *txring,
        int (*build_header)(char *buff, int buff_len, const netmap_address_t *addr),
        const char *buff, int data_len, const netmap_address_t *addr)
{
    assert(txring != NULL);
    assert(buff != NULL);
    assert(data_len > 0);
    assert(addr != NULL);
    assert(build_header != NULL);

    u_int k = txring->cur; /* TX */
    struct netmap_slot *ts = &txring->slot[k];
    char *txbuf = NETMAP_BUF(txring, ts->buf_idx);

    assert(data_len > 0);

    ts->len = data_len + PROTO_LEN; 
    memcpy(txbuf + PROTO_LEN, buff, data_len);

    build_header(txbuf, ts->len, addr);

    k = NETMAP_RING_NEXT(txring, k);

    txring->avail -= 1;
    txring->cur = k;

    return (data_len);
}

int netmap_recv_raw(const netmapio_ringobj *obj, 
        char *buff, int buff_len, netmap_address_t *addr)
{
    struct netmap_ring *rxring;
    u_int ri = obj->begin;
    int ret = NM_R_FAILED;

    while (ri < obj->end) 
    {
        rxring = NETMAP_RXRING(obj->nifp, ri);
        if (rxring->avail == 0) 
        {
            ri ++;
            continue;
        }

        ret = process_recv_from_ring(rxring,
                obj->filter, obj->parse_header, 
                buff, buff_len, addr);
        if (ret > 0) 
        {
            break;
        }

        ri ++;
    }

    return ret;
}

int netmap_send_raw(const netmapio_ringobj *obj, 
        const char *buff, int data_len, const netmap_address_t *addr)
{
    struct netmap_ring *txring;
    u_int ti=obj->begin;
    int ret = NM_R_FAILED;
    
    ti=obj->begin;
    while (ti < obj->end) {
        txring = NETMAP_TXRING(obj->nifp, ti);

        if (txring->avail == 0) 
        {
            ti++;
            continue;
        }   

        ret = process_send_to_ring(txring, obj->build_header, buff, data_len, addr);
        if (ret > 0) 
        {
            break;
        }

        ti ++;
    }

    return ret;
}

