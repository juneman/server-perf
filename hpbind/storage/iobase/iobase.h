/**
 * File: iobase.h
 *
 * author: db
 */

#ifndef __IOBASE_H
#define __IOBASE_H

#include "config.h"
#include "types.h"
#include "log.h"
#include "netmapio.h"

#define O_NOATIME 01000000
/**
 *
 */
#define MAX_FDS (1024)
#define MAX_INTERFACE_NUMS 8

#define NM_QUEUE_CAPCITY (1024 * 2)
#define NM_IFNAME_SIZE (8)

#define NM_PIPELINE_MAGIC ((int)('a' << 24 | 'b' << 16 | 'c' << 8 | 'd'))
#define NM_PIPELINE_VALID(m)  ( (m)->magic == NM_PIPELINE_MAGIC ? 1:0 )

#define CHECK_PIPELINE_AND_EXIT(m) do { \
        assert(NM_PIPELINE_VALID((m))); \
        if (NM_PIPELINE_VALID((m)) == 0) { \
            printf("%s, %s, %d: pipeline broken.\n", \
                    __FILE__, __FUNCTION__, __LINE__); \
            fflush(stdout); \
            exit(1); \
        }\
    }while(0)

//--------------------------------------------
#define iobase_address_t netmap_address_t 

/// for netmap fd
int iobase_init(void);
int iobase_destroy(void);
void iobase_setup(void);
void iobase_teardown(void);
void iobase_terminal(void);

int iobase_openfd(const char *ifname);
int iobase_closefd(int fd);

// fd : netmap file descriptor
// buff: to store recv data from netmap, must NOT NULL
// buff_len: buff's size 
// addr: packet addr info, if NULL, cannt get address from packt
// return: actual recv data length, if <= 0 , occured an error. 
int iobase_recv(int fd, char *buff, int buff_len, iobase_address_t *addr);


// fd : netmap file descriptor
// buff: need to send buff, MUST NOT NULL
// data_len: length of data,  MUST > 0 ...
// addr: packet addr info, MUST NOT NULL
// return: actual send data length, if <= 0, occured an error.
int iobase_send(int fd, const char *buff, int data_len, const iobase_address_t *addr);

//
// while HAS_NOTIFY is not defined or  has_notify is false 
// call this in main thread.
void iobase_loop(void);

/**
 * 
 */
#ifdef IOBASE_HAS_NOTIFY
#define IOBASE_ATTR_NOTIFY 0x000001
// ---------------------------------
#define IOBASE_NOTITY_ON NM_TRUE
#define IOBASE_NOTITY_OFF NM_FALSE
#endif
// call it after iobase_init
void iobase_set_attr(int what, int cmd);


#endif // end of __IOBASE_H
