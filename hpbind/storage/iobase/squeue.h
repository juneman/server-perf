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

#if defined(SQUEUE_USE_LFDS)
#include "liblfds611.h"
#endif

typedef struct __squeue_t__
{
#ifdef SQUEUE_USE_SLIST    
    slock_t lock;
    slist_node_t queue;
    int size;
#elif defined(SQUEUE_USE_LFDS)
    struct lfds611_queue_state *qs;
    volatile int size;
#endif
    int capcity;
}squeue_t;

int squeue_init(squeue_t *sq, int capcity, int size);
#ifdef SQUEUE_USE_SLIST    
sdata_t * squeue_trydeq(squeue_t *sq);
#endif
sdata_t * squeue_deq(squeue_t *sq);
int squeue_enq(squeue_t *sq, sdata_t *data);

void  squeue_use(squeue_t *sq);
int squeue_empty(squeue_t *sq);
int squeue_destroy(squeue_t *sq);

#endif
