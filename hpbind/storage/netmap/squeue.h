/***
 *
 * file: squeue.h
 *      implement a FIFO queue
 *
 * author: db
 * date: 2014-01-17
 */ 

#ifndef __SQUEUE_H_
#define __SQUEUE_H_

#include "slist.h"
#include "slock.h"

typedef struct __sqmsg_t__
{
#define SQMSG_TYPE_UNKNOWN (1)
#define SQMSG_TYPE_RECV    (2)
#define SQMSG_TYPE_SEND    (3)
    unsigned char type;
    unsigned char dirty;
    short refs;

#define SQMSG_SIZE_MAX (511)
    char buff[SQMSG_SIZE_MAX + 1]; 
    int len;

#define SQMSG_EXT_BUFF_SIZE (511)
    char extbuff[SQMSG_EXT_BUFF_SIZE + 1];
    slist_node_t node;
}sqmsg_t;

typedef struct __squeue_t__
{
    slock_t lock;
    scond_t cond;
    slist_node_t queue;
}squeue_t;

inline int sqmsg_init(sqmsg_t *msg);

inline int squeue_init(squeue_t *sq);
inline int squeue_prealloc(squeue_t *sq, int capcity);
inline sqmsg_t * squeue_pop(squeue_t *sq);
inline int squeue_push(squeue_t *sq, sqmsg_t *msg);
inline int squeue_signal(squeue_t *sq);
inline int squeue_wait(squeue_t *sq);

inline int squeue_empty(squeue_t *sq);
inline int squeue_cleanup(squeue_t *sq);

#endif
