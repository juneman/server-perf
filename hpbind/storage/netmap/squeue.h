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
#include "sdata.h"
#include "slock.h"

typedef struct __squeue_t__
{
    slock_t lock;
    scond_t cond;
    slist_node_t queue;
}squeue_t;

inline int squeue_init(squeue_t *sq);
inline int squeue_prealloc(squeue_t *sq, int capcity);
inline sdata_t * squeue_pop(squeue_t *sq);
inline int squeue_push(squeue_t *sq, sdata_t *data);
inline int squeue_signal(squeue_t *sq);
inline int squeue_wait(squeue_t *sq);

inline int squeue_empty(squeue_t *sq);
inline int squeue_cleanup(squeue_t *sq);

#endif
