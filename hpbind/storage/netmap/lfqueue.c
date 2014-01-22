/***
 *
 * file: lfqueue.c
 *      implement a lock-free circular queue 
 *
 * author: db
 * date: 2014-01-22
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sdata.h"
#include "lfqueue.h"

// call in one-thread
inline int lfqueue_init(lfqueue_t *q, int capcity)
{
    assert(q != NULL);
    assert(capcity > 1);

    q->capcity = capcity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    
    q->__buff__ = (sdata_t *)malloc(sizeof(sdata_t) * capcity);
    assert(q->__buff__ != NULL);
    if (NULL == q->__buff__) return 1;
    memset(q->__buff__, 0x0, sizeof(sdata_t) * capcity);

    q->__flags__ = (char*)malloc(capcity);
    assert(q->__flags__ != NULL);
    if (NULL == q->__flags__) 
    {
        free(q->__buff__);
        return 1;
    }
    memset(q->__flags__, 0x0, capcity);

    return 0;
}

inline int lfqueue_dequeue(lfqueue_t *q, 
        char *buff, int buff_len, char *extbuff, int extlen)
{
    assert(q != NULL);
    assert(buff != NULL);
    assert(extbuff != NULL);
    assert(buff_len > 0);

    int n = -1;

    if (q->count <= 0) return (-1);
    
    volatile int cur_head = q->head;
    char *cur_head_flag = q->__flags__ + cur_head;

    register long long times = 10;
    while(! __sync_bool_compare_and_swap(cur_head_flag, LFQ_SLOT_WDONE, LFQ_SLOT_READING)) 
    {
        cur_head = q->head;
        cur_head_flag = q->__flags__ + cur_head;
        if (times <= 1) return (-1);
        times --;
        sleep(0);
    }

    volatile int update_head = (cur_head+1) & (q->capcity - 1);
    __sync_bool_compare_and_swap(&(q->head), cur_head, update_head);

    sdata_t *data = (((sdata_t*)q->__buff__) + cur_head);

    memcpy(buff, data->buff, data->len);
    memcpy(extbuff, data->extbuff, extlen);
    n = data->len;

    __sync_fetch_and_sub(cur_head_flag, 3);
    __sync_fetch_and_sub(&(q->count), 1);
    
    return (n);
}

inline int lfqueue_enqueue(lfqueue_t *q, 
        const char *buff, int data_len, const char *extbuff, int extlen) 
{
    assert(q != NULL);
    assert(buff  != NULL);
    assert(extbuff  != NULL);
    assert(data_len > 0 );
    assert(data_len < SDATA_SIZE_MAX);
    assert(extlen > 0);
    assert(extlen < SDATA_EXT_BUFF_SIZE);
    
    if (q->count >= q->capcity) return (-1);

    volatile int cur_tail = q->tail;
    char *cur_tail_flag = q->__flags__ + cur_tail;
    
    register long long times = 1000;
    // wait busy
    while (!__sync_bool_compare_and_swap(cur_tail_flag, 
                LFQ_SLOT_IDLE, LFQ_SLOT_WRITING))
    {
        cur_tail = q->tail;
        cur_tail_flag = q->__flags__ + cur_tail;
        if (times <= 1) return (-1);
        times --;
        sleep(0);
    }

    volatile int update_tail = (cur_tail+1) & (q->capcity - 1);

    __sync_bool_compare_and_swap(&(q->tail), cur_tail, update_tail);
    sdata_t *data = (((sdata_t *)q->__buff__) + cur_tail);
    memcpy(data->buff, buff, data_len);
    memcpy(data->extbuff, extbuff, extlen);
    data->len = data_len;

    __sync_fetch_and_add(cur_tail_flag, 1);
    __sync_fetch_and_add(&(q->count), 1);

    return (data_len);
}

inline void lfqueue_destroy(lfqueue_t *q)
{
    assert(q != NULL);
    
    free(q->__buff__);
    free(q->__flags__);

    q->__buff__ = NULL;
    q->__flags__ = NULL;
    q->capcity = 0;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
}
