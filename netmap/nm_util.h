
#ifndef _NM_UTIL_H
#define _NM_UTIL_H
#include <errno.h>
#include <signal.h>	/* signal */
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>	/* PRI* macros */
#include <string.h>	/* strcmp */
#include <fcntl.h>	/* open */
#include <unistd.h>	/* close */
#include <ifaddrs.h>	/* getifaddrs */

#include <sys/mman.h>	/* PROT_* */
#include <sys/ioctl.h>	/* ioctl */
#include <sys/poll.h>
#include <sys/socket.h>	/* sockaddr.. */
#include <arpa/inet.h>	/* ntohs */
#include <sys/param.h>
#include <sys/sysctl.h>	/* sysctl */
#include <sys/time.h>	/* timersub */

#include <net/ethernet.h>
#include <net/if.h>	/* ifreq */

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "net/netmap.h"
#include "net/netmap_user.h"

#include <pthread.h>	/* pthread_* */

#ifdef linux
#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC
#include <linux/ethtool.h>
#include <linux/sockios.h>

#define CLOCK_REALTIME_PRECISE CLOCK_REALTIME
#include <netinet/ether.h>      /* ether_aton */
#include <linux/if_packet.h>    /* sockaddr_ll */
#endif	/* linux */

static inline int min(int a, int b) { return a < b ? a : b; }
extern int time_second;

/* debug support */
#define ND(format, ...)	do {} while(0)
#define D(format, ...)					\
	fprintf(stderr, "%s [%d] " format "\n",		\
	__FUNCTION__, __LINE__, ##__VA_ARGS__)

#define RD(lps, format, ...)				\
	do {						\
		static int t0, cnt;			\
		if (t0 != time_second) {		\
			t0 = time_second;		\
			cnt = 0;			\
		}					\
		if (cnt++ < lps)			\
			D(format, ##__VA_ARGS__);	\
	} while (0)



// XXX does it work on 32-bit machines ?
static inline void prefetch (const void *x)
{
	__asm volatile("prefetcht0 %0" :: "m" (*(const unsigned long *)x));
}

// XXX only for multiples of 64 bytes, non overlapped.
static inline void
pkt_copy(const void *_src, void *_dst, int l)
{
	const uint64_t *src = _src;
	uint64_t *dst = _dst;
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)       __builtin_expect(!!(x), 0)
	if (unlikely(l >= 1024)) {
		bcopy(src, dst, l);
		return;
	}
	for (; l > 0; l-=64) {
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
	}
}

//#define NM_HAVE_G_LOCK 
//#define NM_HAVE_MRING_LOCK
#define NM_HAVE_RING_LOCK

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

#ifdef NM_HAVE_MRING_LOCK    
    pthread_mutex_t rxlock;
    pthread_mutex_t txlock;
#endif

	uint32_t if_flags;
	uint32_t if_reqcap;
	uint32_t if_curcap;
};

int netmap_getfd(const char *ifname);
int netmap_closefd(int fd);
struct my_ring *netmap_getring(int fd);

#endif /* _NM_UTIL_H */
