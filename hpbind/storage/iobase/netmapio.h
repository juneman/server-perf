/*
 * File: netmapio.h
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef _NETMAPIO_H
#define _NETMAPIO_H
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

#include "netmap.h"
#include "netmap_user.h"

#define ifr_flagshigh  ifr_flags
#define ifr_curcap     ifr_flags
#define ifr_reqcap     ifr_flags
#define IFF_PPROMISC   IFF_PROMISC
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "config.h"
#include "types.h"

/*
 *
 * packet address info
 *
 */ 
typedef struct __netmap_address_t__ {
    unsigned char local_macaddr[6];
    unsigned char remote_macaddr[6];
    unsigned short local_port;
    unsigned short remote_port;
    unsigned int local_addr;
    unsigned int remote_addr;
}netmap_address_t;

/*
 * info on a ring we handle
 */
typedef struct __netmapio_ringobj__ {
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

    int (*parse_header)(const char *buff, int len, netmap_address_t * addr);
    int (*build_header)(char *buff, int len, const netmap_address_t *addr);
    NM_BOOL (*filter)(const char *buff, int len);
}netmapio_ringobj;

/**
 * return file desc if success, if < 0 an error occured.
 */
int netmap_open(netmapio_ringobj *obj, int ringid, int promisc);

/*
 * return : NM_R_SUCCESS is closed , other is an error occured. 
 */
int netmap_close(netmapio_ringobj *obj);

/*
 * buff: to store recv data, the caller provide and free. 
 *          MUST NOT NULL
 * buff_len : length of the buff. MUST > 0
 * addr: to store packet' s mac/ip/port etc.. MUST NOT NULL
 *
 * return: return bytes received. if <= 0, an error occured.
 */
int netmap_recv_raw(const netmapio_ringobj *obj, 
        char *buff, int buff_len, netmap_address_t *addr);

/*
 * buff: the data we need to send, the caller provide and free.
 *          MUST NOT NULL
 * data_len : valid length of the data in the buff. MUST > 0
 * addr: to tell the driver the packet' s mac/ip/port etc..MUST NOT NULL.
 *              
 * return: return bytes sended. if <= 0, an error occured.
 */
int netmap_send_raw(const netmapio_ringobj *obj, 
        const char *buff, int data_len, const netmap_address_t *addr);
/**
 *  if use NETMAP_NO_TX_POLL in netmap_open,
 *  we should to call this function by ourself.
 *
 */
void netmap_tx_force(const netmapio_ringobj *obj);

#endif /* _NMIO_H */
