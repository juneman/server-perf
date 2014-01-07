/**
 * File: iobase.h
 *
 * author: db
 */

#ifndef __IOBASE_H
#define __IOBASE_H

#include "config.h"
#include "nm_util.h"
#include "dns_util.h"
#include "build.h"

#define PROTO_LEN (14 + 20 + 8)

#define IO_BLOCK_BUFF_SIZE 512 
typedef struct __io_block_t__ {
    unsigned char local_macaddr[6];
    unsigned char remote_macaddr[6];
    unsigned short local_port;
    unsigned short remote_port;
    unsigned int local_addr;
    unsigned int remote_addr;
    int data_len; // read/write n bytes from/to netmap
    char buffer[IO_BLOCK_BUFF_SIZE];  // read/write buffer
}io_block_t;

#define IO_CACHE_BLOCKS_MAX (256 * 8)
typedef struct __io_cache_t__ {
    int writep; // next block to write
    int readp; // next block to read
    int capcity; // array of blocks 's capcity 
    io_block_t *blocks; // pointer to an array
}io_cache_t;

#define CACHE_GET_WRITEABLE_BLOCK(cache) \
    ((cache)->blocks + (cache)->writep)

#define CACHE_FLUSH_W(cache) do { (cache)->writep ++; (cache)->writep %= (cache)->capcity; }while(0)

#define CACHE_GET_READABLE_BLOCK(cache) \
    ((cache)->blocks + (cache)->readp)

#define CACHE_FLUSH_R(cache) do {(cache)->readp ++; (cache)->readp %= (cache)->capcity; } while(0)

#define CACHE_RESET(cache) do { \
    memset((cache)->blocks, 0x0, sizeof(io_block_t) * (cache)->capcity); \
    (cache)->writep = 0; \
    (cache)->readp = 0; \
        }while(0) 

#define IS_CACHE_FULL(cache) \
    (((((cache)->writep + 1) % (cache)->capcity) == (cache)->readp) ? 1:0)  

#define IS_CACHE_EMPTY(cache) \
    (((cache)->writep == (cache)->readp) ? 1 : 0)

/// for netmap fd
int netmap_init(void *arg);
int netmap_getfd(const char *ifname);
int netmap_closefd(int fd);

// for base io
int netmap_recv(int fd, io_cache_t *cache);
int netmap_recv2(int fd, io_block_t *block);
int netmap_send(int fd, io_cache_t *cache);
int netmap_send2(int fd, io_block_t *block);

//int netmap_flush(int fd, io_cache_t *cache);

#endif // end of __IOBASE_H
