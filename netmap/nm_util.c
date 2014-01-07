/**
 * File: nm_util.c
 *
 * author: db
 *
 */
#include "nm_util.h"

static int verbose = 0;
static pthread_mutex_t g_lock_lock = PTHREAD_MUTEX_INITIALIZER;

int nm_do_ioctl(struct my_ring *me, u_long what, int subcmd)
{
    struct ifreq ifr;
    int error;

    struct ethtool_value eval;
    int fd; 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("Error: cannot get device control socket.\n");
        return -1;
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
                D("flags are 0x%x", me->if_flags);
            break;
    }
done:
    close(fd);
    if (error)
        D("ioctl error %d %lu", error, what);
    return error;
}

/*
 * open a device. if me->mem is null then do an mmap.
 * Returns the file descriptor.
 * The extra flag checks configures promisc mode.
 */
int netmap_open(struct my_ring *me, int ringid, int promisc)
{
    int fd, err, l;
    struct nmreq req;

    me->fd = fd = open("/dev/netmap", O_RDWR);
    if (fd < 0) {
        D("Unable to open /dev/netmap");
        return (-1);
    }
    bzero(&req, sizeof(req));
    req.nr_version = NETMAP_API;
    strncpy(req.nr_name, me->ifname, sizeof(req.nr_name));
    req.nr_ringid = ringid;
    err = ioctl(fd, NIOCGINFO, &req);
    if (err) {
        D("cannot get info on %s, errno %d ver %d",
                me->ifname, errno, req.nr_version);
        goto error;
    }
    me->memsize = l = req.nr_memsize;
    if (verbose)
        D("memsize is %d MB", l>>20);
    err = ioctl(fd, NIOCREGIF, &req);
    if (err) {
        D("Unable to register %s", me->ifname);
        goto error;
    }

    if (me->mem == NULL) {
        me->mem = mmap(0, l, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
        if (me->mem == MAP_FAILED) {
            D("Unable to mmap");
            me->mem = NULL;
            goto error;
        }
    }

    /* Set the operating mode. */
    if (ringid != NETMAP_SW_RING) {
        nm_do_ioctl(me, SIOCGIFFLAGS, 0);
        if ((me[0].if_flags & IFF_UP) == 0) {
            D("%s is down, bringing up...", me[0].ifname);
            me[0].if_flags |= IFF_UP;
        }
        if (promisc) {
            me[0].if_flags |= IFF_PPROMISC;
            nm_do_ioctl(me, SIOCSIFFLAGS, 0);
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
        D("XXX check multiple threads");
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
    return (0);
error:
    close(me->fd);
    return -1;
}

int netmap_close(struct my_ring *me)
{
    D("");
    if (me->mem)
        munmap(me->mem, me->memsize);
    close(me->fd);
    return (0);
}

//------------------------------------------//
///////
// lock impl
int netmap_lock_init(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_init(&(lock->lock), NULL);
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_init(&(lock->lock), PTHREAD_PROCESS_SHARED); 
#else
    assert(0);
#endif

}
int netmap_lock_destroy(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_destroy(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_destroy(&(lock->lock));
#else
    assert(0);
#endif

}
int netmap_lock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_lock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_lock(&(lock->lock));
#else
    assert(0);
#endif

}

int netmap_trylock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_trylock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_trylock(&(lock->lock));
#else
    assert(0);
#endif

}
int netmap_unlock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_unlock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_unlock(&(lock->lock));
#else
    assert(0);
#endif
}


