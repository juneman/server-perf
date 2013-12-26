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
#include "build.h"

typedef struct _io_msg_s_ {
    char *buff;  // read/write buffer
    int buff_len; // length of buff
    int n; // read/write n bytes from/to netmap
    unsigned int local_addr;
    unsigned int remote_addr;
    unsigned short local_port;
    unsigned short remote_port;
    unsigned char local_macaddr[6];
    unsigned char remote_macaddr[6];

}io_msg_s;

int netmap_init();
int netmap_recv(int fd, io_msg_s *iomsg);
int netmap_send(int fd, io_msg_s *iomsg);

#endif // end of __DNS_ECHO_H
