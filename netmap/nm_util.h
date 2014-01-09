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
#include <unistd.h>
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

int netmap_open(struct my_ring *me, int ringid, int promisc);
int netmap_close(struct my_ring *me);

#endif /* _NM_UTIL_H */
