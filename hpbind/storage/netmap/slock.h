/*
 * File: slock.h : sample lock
 * Date: 2013-12-25
 * 
 * author: db
 * 
 *
 */
#ifndef __LOCK_H
#define __LOCK_H
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>	/* pthread_* */

#define slock_t pthread_mutex_t

#define slock_init(lock) \
    pthread_mutex_init(lock, NULL)

#define slock_destroy(lock) \
    pthread_mutex_destroy(lock)

#define slock_lock(lock) \
    pthread_mutex_lock(lock)

#define slock_trylock(lock) \
    pthread_mutex_trylock(lock)

#define slock_unlock(lock) \
    pthread_mutex_unlock(lock)

typedef struct __scond_t__
{
    pthread_cond_t c;
    pthread_mutex_t m;
}scond_t;

#define scond_init(cond) do {\
    pthread_cond_init(&((cond)->c), NULL); \
    pthread_mutex_init(&((cond)->m), NULL); }while(0)

#define scond_signal(cond) \
    pthread_cond_signal(&((cond)->c))

#define scond_broadcast(cond) \
    pthread_cond_broadcast(&((cond)->c))

#define scond_wait(cond) \
    pthread_cond_wait(&((cond)->c), &((cond)->m))

#define scond_destroy(cond) do{ \
    pthread_cond_destroy(&((cond)->c)); \
    pthread_mutex_destroy(&((cond)->m));}while(0)


#endif /* __LOCK_H */
