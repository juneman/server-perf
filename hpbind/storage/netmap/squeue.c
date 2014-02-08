/***
 *
 * file: squeue.c
 *      implement a FIFO queue
 *
 * author: db
 * date: 2014-01-17
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "squeue.h"

inline int squeue_init(squeue_t *sq, int capcity, int size, int attrs)
{
    assert(sq != NULL);
    assert(capcity > 0);
    assert(size >= 0);
    assert(capcity >= size);

    int index = 0;
    
    slock_init(&(sq->lock));
    slist_init(&(sq->queue));
   
    slist_node_t *list = &(sq->queue);

    for (index = 0; index < size; index ++)
    {
        sdata_t *data = (sdata_t *) malloc(sizeof(sdata_t));
        assert(data != NULL);
        if (NULL == data) 
        {
            squeue_destroy(sq);
            printf("malloc failed.\n");
            exit(1);
        }
        
        sdata_init(data);
        slist_add(list, &(data->node));
    }
    
    sq->capcity = capcity;
    sq->size = size;
    sq->attrs = attrs;
    sq->unactived = 0;    
    return 0;
}

inline int squeue_empty(squeue_t *sq)
{
    return __sync_bool_compare_and_swap(&(sq->size), 0, 0);
}

inline int squeue_full(squeue_t *sq)
{
    int capcity = sq->capcity;
    return __sync_bool_compare_and_swap(&(sq->size), capcity, capcity);
}

inline sdata_t * squeue_pop(squeue_t *sq)
{
    assert(sq != NULL);
    sdata_t *data = NULL;
    int needlock = 0;
    
    if (sq->unactived == 1)
        return NULL;

    if (sq->attrs & SQUEUE_SIG_READ) 
    {
        if (__sync_bool_compare_and_swap(&(sq->size), 0, 0)) return NULL;
    }
    else
    {
        needlock = 1;
    } 

    if (needlock)
    {
        slock_lock(&(sq->lock));
        if (slist_empty(&(sq->queue))) goto END_L;
    }
    
    slist_node_t *node = sq->queue.next;
    data = slist_entry(node, sdata_t, node);
    slist_delete(node);
    
    __sync_fetch_and_sub(&(sq->size), 1);

END_L: 

    if (needlock) slock_unlock(&(sq->lock));

    return data;
}

inline int squeue_push(squeue_t *sq, sdata_t *data)
{
    assert(sq != NULL);
    assert(data != NULL);
    
    int needlock = 0;

    if (sq->unactived == 1)
        return 0;

    if (sq->attrs & SQUEUE_SIG_WRITE) 
    {
        if (__sync_bool_compare_and_swap(&(sq->size), 0, 0)) 
            needlock = 1;
    }
    else
    {
        needlock = 1;
    }
    
    if (needlock) slock_lock(&(sq->lock)); 
        
    slist_add_tail(&(sq->queue), &(data->node));
    __sync_fetch_and_add(&(sq->size), 1);

    if (needlock) slock_unlock(&(sq->lock));

    return 0;
}

inline int squeue_destroy(squeue_t *sq)
{
    sdata_t *data = NULL;
    sdata_t *temp = NULL;
    
    slock_lock(&(sq->lock));

    if (sq->unactived == 1)
        goto END; 

    sq->unactived = 1;
     
    slist_foreach_entry_safe(data, temp, &(sq->queue), sdata_t, node)
    {
        slist_delete(&(data->node));
        free(data);
        __sync_fetch_and_sub(&(sq->size), 1);
    }

    assert(squeue_empty(sq));
     
END:
    slock_unlock(&(sq->lock));

    return 0;
}


