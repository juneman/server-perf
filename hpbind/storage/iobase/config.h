/*
 * File: config.h
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef _CONFIG_H
#define _CONFIG_H

// for enable/disable debug
#define NM_DEBUG

// for debug parse/build packets
//#define __DEBUG_VERB__

// sync epoll_wait and send in
// iobase lib
//#define IOBASE_IOSYNC_USE_CAS

// if defined it, user can use 
// epoll to listen the fd returned by /iobase_openfd/
#define IOBASE_HAS_NOTIFY

// report recv/send packet nums
#define IOBASE_HAS_REPORT

// cbuff
//#define CBUFF_USE_LOCK

// squeue
// 
//#define SQUEUE_USE_LFDS
//#define SQUEUE_USE_SLIST


#endif /* _CONFIG_H */
