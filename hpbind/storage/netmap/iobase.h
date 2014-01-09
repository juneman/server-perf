/**
 * File: iobase.h
 *
 * author: db
 */

#ifndef __IOBASE_H
#define __IOBASE_H

#include "config.h"
#include "types.h"
#include "nm_util.h"
#include "nm_lock.h"
#include "dns_util.h"
#include "build.h"

#define PROTO_LEN (14 + 20 + 8)

#define NM_PKT_BUFF_SIZE_MAX 512 
typedef struct __netmap_address_t__ {
    unsigned char local_macaddr[6];
    unsigned char remote_macaddr[6];
    unsigned short local_port;
    unsigned short remote_port;
    unsigned int local_addr;
    unsigned int remote_addr;
}netmap_address_t;

/// for netmap fd
int netmap_init(void);
int netmap_destroy(void);
int netmap_openfd(const char *ifname);
int netmap_closefd(int fd);

// fd : netmap file descriptor
// buff: to store recv data from netmap, must NOT NULL
// data_len: to store length of recv data, MUST NOT NULL
// addr: packet addr info, if NULL, cannt get address from packt
// return 
int netmap_recv(int fd, char *buff, int *data_len, netmap_address_t *addr);


// fd : netmap file descriptor
// buff: need to send buff, MUST NOT NULL
// data_len: length of data,  MUST > 0 ...
// addr: packet addr info, MUST NOT NULL
// return: 0, send ok. !0 send error.
int netmap_send(int fd, char *buff, int data_len, netmap_address_t *addr);

#endif // end of __IOBASE_H
