/*
 * File: nm_lock.h
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef _NM_LOCK_H
#define _NM_LOCK_H
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>	/* pthread_* */

#include "config.h"
#include "nm_util.h"

// lock wraper
typedef struct __netmap_lock_t__
{
#ifdef NM_USE_MUTEX_LOCK
    pthread_mutex_t lock;
#else
    pthread_spinlock_t lock;
#endif
}netmap_lock_t;

int netmap_lock_init(netmap_lock_t *lock);
int netmap_lock_destory(netmap_lock_t *lock);
int netmap_lock(netmap_lock_t *lock);
int netmap_trylock(netmap_lock_t *lock);
int netmap_unlock(netmap_lock_t *lock);


#endif /* _NM_LOCK_H */
