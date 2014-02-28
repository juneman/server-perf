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


#ifdef _SQ_USE_SLIST
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

inline sdata_t * squeue_trydeq(squeue_t *sq)
{
    assert(sq != NULL);
    sdata_t *data = NULL;
    
    int needlock = 0;

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
	
	{
    	slist_node_t *node = sq->queue.next;
    	data = slist_entry(node, sdata_t, node);
        CHECK_SDATA_AND_EXIT(data);
	}

END_L: 

    if (needlock) slock_unlock(&(sq->lock));

    return data;
}

inline sdata_t * squeue_deq(squeue_t *sq)
{
    assert(sq != NULL);
    sdata_t *data = NULL;
    
	slock_lock(&(sq->lock));

	if (squeue_empty(sq)) goto END_L;

	slist_node_t *node = sq->queue.next;
	data = slist_entry(node, sdata_t, node);
    CHECK_SDATA_AND_EXIT(data);
	slist_delete(node);

	sq->size --;

END_L: 
    slock_unlock(&(sq->lock));

    return data;
}

inline int squeue_enq(squeue_t *sq, sdata_t *data)
{
    assert(sq != NULL);
    assert(data != NULL);
    
    CHECK_SDATA_AND_EXIT(data);

    slock_lock(&(sq->lock)); 

    slist_add_tail(&(sq->queue), &(data->node));
    sq->size++;

    slock_unlock(&(sq->lock));

    return 0;
}

inline int squeue_destroy(squeue_t *sq)
{
    sdata_t *data = NULL;
    sdata_t *temp = NULL;
    
    slock_lock(&(sq->lock));

    slist_foreach_entry_safe(data, temp, &(sq->queue), sdata_t, node)
    {
        CHECK_SDATA_AND_EXIT(data);
        slist_delete(&(data->node));
        free(data);
        __sync_fetch_and_sub(&(sq->size), 1);
    }

    assert(squeue_empty(sq));
     
    slock_unlock(&(sq->lock));

    return 0;
}

#elif defined(_SQ_USE_LFDS)

inline int squeue_init(squeue_t *sq, int capcity, int size, int attrs)
{
    assert(sq != NULL);
    assert(capcity > 0);
    assert(size >= 0);
    assert(capcity >= size);
    
    (void)attrs;

    sq->qs = NULL;
    sq->capcity = capcity;
    sq->size = (volatile int)size;
    sq->attrs = 0;
    
    lfds611_queue_new(&(sq->qs), capcity);
    
    assert(sq->qs != NULL);
    if (sq->qs == NULL) return -1; 
    
    if (size == 0) return 0;

    int index = 0;
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
        lfds611_queue_guaranteed_enqueue(sq->qs, (void*)data);
    }
    
    return 0;
}

inline sdata_t * squeue_trydeq(squeue_t *sq)
{
    return squeue_deq(sq);
}

inline void squeue_use(squeue_t *sq)
{
    assert(sq != NULL);
    assert(sq->qs != NULL);

    lfds611_queue_use(sq->qs);
}

inline sdata_t * squeue_deq(squeue_t *sq)
{
    assert(sq != NULL);
    assert(sq->qs != NULL);

    sdata_t *data = NULL;

    lfds611_queue_dequeue(sq->qs, (void *)&data);
    
    if (data != NULL) 
        __sync_fetch_and_sub(&sq->size, 1);

    return data;
}

inline int squeue_enq(squeue_t *sq, sdata_t *data)
{
    assert(sq != NULL);
    assert(sq->qs != NULL);
    assert(data != NULL);
    
    //lfds611_queue_guaranteed_enqueue(sq->qs, (void *)data);
    lfds611_queue_enqueue(sq->qs, (void *)data);
    __sync_fetch_and_add(&(sq->size), 1);

    return 0;
}

inline int squeue_empty(squeue_t *sq)
{
    assert(sq != NULL);
    if (__sync_bool_compare_and_swap(&(sq->size), 0, 0)) 
        return 1;

    return 0;
}

static inline void squeue_free_data(void *data, void *sq)
{
    assert(data != NULL);
    assert(sq != NULL);
    
    free(data);
    __sync_fetch_and_sub(&(((squeue_t*)sq)->size), 1);
}

inline int squeue_destroy(squeue_t *sq)
{
    assert(sq != NULL);
    assert(sq->qs != NULL);

    lfds611_queue_delete(sq->qs, squeue_free_data, sq);
    assert(squeue_empty(sq));

    return 0;
}

#endif
