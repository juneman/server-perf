#ifndef __DNS_ECHO_H
#define __DNS_ECHO_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <sys/mman.h> /* PROT_* */
#include <sys/ioctl.h> /* ioctl */
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h> /* sockaddr.. */
#include <arpa/inet.h> /* ntohs */

#include <sys/epoll.h>

#include <net/if.h>	/* ifreq */
#include <net/ethernet.h>

#include <netinet/if_ether.h>
#include <netinet/in.h> /* sockaddr_in */
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "net/netmap.h"
#include "net/netmap_user.h"

#include "nm_util.h"

typedef struct _io_msg_s_ {
    char *buff;  // read/write buffer
    int buff_len; // length of buff
    int n; // read/write n bytes from/to netmap
    unsigned int saddr;
    unsigned int daddr;
    unsigned short source;
    unsigned short dest;
}io_msg_s;

typedef struct _macaddr_map_s_ {
    char smac[6]; // local mac addr
    char dmac[6]; // remote mac addr
    char dirty;
    char pad1;
    unsigned short qid; // dns query id
    unsigned int saddr; // local ip addr
    unsigned short source; // local udp port
    char pad2[2];
}macaddr_map_s;

int netmap_recv(int fd, io_msg_s *iomsg);
int netmap_send(int fd, io_msg_s *iomsg);

#endif // end of __DNS_ECHO_H
