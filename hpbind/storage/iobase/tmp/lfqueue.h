/***
 *
 * file: lfqueue.h
 *      implement a lock-free circular-queue
 *
 * author: db
 * date: 2014-01-22
 */ 

#ifndef __LFQUEUE_H_
#define __LFQUEUE_H_

#include "sdata.h"

typedef struct __lfqueue_t__
{
    sdata_t *__buff__;
#define LFQ_SLOT_IDLE (0)
#define LFQ_SLOT_WRITING (1)
#define LFQ_SLOT_WDONE (2)
#define LFQ_SLOT_READING (3)

    char *__flags__; //array: 
                     // each element indicate:
                     // 0: slot is idle, 1: busy, writing by someone
                     // 2: writed done, available for read, 3: reading
    volatile int capcity;
    volatile int count;
    volatile int head;
    volatile int tail;
}lfqueue_t;

inline int lfqueue_init(lfqueue_t *q, int capcity);

// return : 0,success, other is error
inline int lfqueue_dequeue(lfqueue_t *q, char *buff, int buff_len, char *extbuff, int extlen); 

// return : 0,success, other is error
inline int lfqueue_enqueue(lfqueue_t *q, const char *buff, int data_len, const char *extbuff, int extlen);

inline void lfqueue_destroy(lfqueue_t *q);

#endif
