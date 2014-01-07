/*
 * File: nm_util.h
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef _NM_UTIL_H
#define _NM_UTIL_H
#include <errno.h>
#include <assert.h>	/* signal */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>	/* strcmp */
#include <fcntl.h>	/* open */
#include <sys/mman.h>	/* PROT_* */
#include <sys/ioctl.h>	/* ioctl */
#include <sys/socket.h>	/* sockaddr.. */
#include <arpa/inet.h>	/* ntohs */
#include <net/if.h>	/* ifreq */
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <pthread.h>	/* pthread_* */

#include "net/netmap.h"
#include "net/netmap_user.h"

#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "config.h"

/* debug support */
#ifdef NM_DEBUG
#define D(format, ...)					\
	fprintf(stderr, "%s [%d] " format "\n",		\
	__FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
#define D(format, ...)	do {} while(0)
#endif

/*
 * info on a ring we handle
 */
struct my_ring {
	const char *ifname;
	int fd;
	char *mem;                      /* userspace mmap address */
	u_int memsize;
	u_int queueid;
	u_int begin, end;               /* first..last+1 rings to check */
	struct netmap_if *nifp;
	struct netmap_ring *tx, *rx;    /* shortcuts */
	uint32_t if_flags;
	uint32_t if_reqcap;
	uint32_t if_curcap;
};

// lock wraper
typedef struct __netmap_lock_t__
{
#ifdef NM_USE_MUTEX_LOCK
    pthread_mutex_t lock;
#else
    pthread_spinlock_t lock;
#endif
}netmap_lock_t;

int netmap_lock_init(netmap_lock_t *lock);
int netmap_lock_destory(netmap_lock_t *lock);
int netmap_lock(netmap_lock_t *lock);
int netmap_trylock(netmap_lock_t *lock);
int netmap_unlock(netmap_lock_t *lock);


#endif /* _NM_UTIL_H */
