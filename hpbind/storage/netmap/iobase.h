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
#include "slock.h"
#include "dns_util.h"
#include "build.h"
#include "squeue.h"

#define NM_PKT_BUFF_SIZE_MAX (SQMSG_SIZE_MAX)
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
void netmap_setup(void);
void netmap_teardown(void);

int netmap_openfd(const char *ifname);
int netmap_closefd(int fd);

// fd : netmap file descriptor
// buff: to store recv data from netmap, must NOT NULL
// buff_len: buff's size 
// addr: packet addr info, if NULL, cannt get address from packt
// return: actual recv data length, if <= 0 , occured an error. 
int netmap_recv(int fd, char *buff, int buff_len, netmap_address_t *addr);


// fd : netmap file descriptor
// buff: need to send buff, MUST NOT NULL
// data_len: length of data,  MUST > 0 ...
// addr: packet addr info, MUST NOT NULL
// return: actual send data length, if <= 0, occured an error.
int netmap_send(int fd, const char *buff, int data_len, const netmap_address_t *addr);

#endif // end of __IOBASE_H
