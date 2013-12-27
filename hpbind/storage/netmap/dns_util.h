/**
 * File: dns_util.h
 *
 * author: db
 */

#ifndef __DNS_ECHO_H
#define __DNS_ECHO_H

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
