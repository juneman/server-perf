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

inline int sqmsg_init(sqmsg_t *msg)
{
    memset(msg, 0x0, sizeof(sqmsg_t));
    slist_init(&(msg->node));

    return 0;
}

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
        sqmsg_t *msg = (sqmsg_t *) malloc(sizeof(sqmsg_t));
        assert(msg != NULL);
        if (NULL == msg) 
        {
            squeue_cleanup(sq);
            printf("malloc failed.\n");
            exit(1);
        }
        
        sqmsg_init(msg);
        slist_add(list, &(msg->node));
    }
    
    return 0;
}

inline int squeue_empty(squeue_t *sq)
{
    return slist_empty(&(sq->queue));
}

inline sqmsg_t * squeue_pop(squeue_t *sq)
{
    assert(sq != NULL);
    sqmsg_t *msg = NULL;

    slock_lock(&(sq->lock));

    if (slist_empty(&(sq->queue))) goto END_L;
    
    slist_node_t *node = sq->queue.next;
    msg = slist_entry(node, sqmsg_t, node);
    slist_delete(node);
    
END_L: 
    slock_unlock(&(sq->lock));

    return msg;
}

inline int squeue_push(squeue_t *sq, sqmsg_t *msg)
{
    assert(sq != NULL);
    assert(msg != NULL);
    
    int empty = 0;

    slock_lock(&(sq->lock));
    empty = slist_empty(&(sq->queue));
    slist_add_tail(&(sq->queue), &(msg->node));
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
    sqmsg_t *msg = NULL;
    sqmsg_t *temp = NULL;
    
    slock_lock(&(sq->lock));
    slist_foreach_entry_safe(msg, temp, &(sq->queue), sqmsg_t, node)
    {
        slist_delete(&(msg->node));
        free(msg);
    }
    assert(squeue_empty(sq));
    slock_unlock(&(sq->lock));

    return 0;
}


