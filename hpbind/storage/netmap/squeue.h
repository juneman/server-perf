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

#define SQUEUE_SIG_READ  (0x00001)
#define SQUEUE_MUL_READ  (0x00010)
#define SQUEUE_SIG_WRITE (0x00100)
#define SQUEUE_MUL_WRITE (0x01000)

typedef struct __squeue_t__
{
    slock_t lock;
    slist_node_t queue;
    int capcity;
    int size;
    int attrs;
    int unactived;
}squeue_t;

inline int squeue_init(squeue_t *sq, int capcity, int size, int attrs);
inline sdata_t * squeue_pop(squeue_t *sq);
inline int squeue_push(squeue_t *sq, sdata_t *data);

inline int squeue_empty(squeue_t *sq);
inline int squeue_destroy(squeue_t *sq);

#endif
