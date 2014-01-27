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


inline int squeue_init(squeue_t *sq)
{
   assert(sq != NULL);

   slock_init(&(sq->lock));
   slist_init(&(sq->queue));
   scond_init(&(sq->cond));
   return 0;
}

inline int squeue_prealloc(squeue_t *sq, int capcity)
{
    int index = 0;
    
    assert(capcity > 0);

    squeue_init(sq);
    slist_node_t *list = &(sq->queue);

    for (index = 0; index < capcity; index ++)
    {
        sdata_t *data = (sdata_t *) malloc(sizeof(sdata_t));
        assert(data != NULL);
        if (NULL == data) 
        {
            squeue_cleanup(sq);
            printf("malloc failed.\n");
            exit(1);
        }
        
        sdata_init(data);
        slist_add(list, &(data->node));
    }
    
    return 0;
}

inline int squeue_empty(squeue_t *sq)
{
    return slist_empty(&(sq->queue));
}

inline sdata_t * squeue_pop(squeue_t *sq)
{
    assert(sq != NULL);
    sdata_t *data = NULL;

    slock_lock(&(sq->lock));

    if (slist_empty(&(sq->queue))) goto END_L;
    
    slist_node_t *node = sq->queue.next;
    data = slist_entry(node, sdata_t, node);
    slist_delete(node);
    
END_L: 
    slock_unlock(&(sq->lock));

    return data;
}

inline int squeue_push(squeue_t *sq, sdata_t *data)
{
    assert(sq != NULL);
    assert(data != NULL);
    
    int empty = 0;

    slock_lock(&(sq->lock));
    empty = slist_empty(&(sq->queue));
    slist_add_tail(&(sq->queue), &(data->node));
    slock_unlock(&(sq->lock));
    
    if (empty) scond_broadcast(&(sq->cond));

    return 0;
}

inline int squeue_wait(squeue_t *sq)
{
    return scond_wait(&(sq->cond));
}

inline int squeue_cleanup(squeue_t *sq)
{
    sdata_t *data = NULL;
    sdata_t *temp = NULL;
    
    slock_lock(&(sq->lock));
    slist_foreach_entry_safe(data, temp, &(sq->queue), sdata_t, node)
    {
        slist_delete(&(data->node));
        free(data);
    }
    assert(squeue_empty(sq));
    slock_unlock(&(sq->lock));

    return 0;
}


