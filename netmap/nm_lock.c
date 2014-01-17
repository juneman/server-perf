/**
 * File: nm_lock.c
 *
 * author: db
 *
 */
#include "nm_lock.h"

//------------------------------------------//
///////
// lock impl
int netmap_lock_init(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_init(&(lock->lock), NULL);
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_init(&(lock->lock), PTHREAD_PROCESS_SHARED); 
#else
    assert(0);
#endif

}

int netmap_lock_destroy(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_destroy(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_destroy(&(lock->lock));
#else
    assert(0);
#endif

}
int netmap_lock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_lock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_lock(&(lock->lock));
#else
    assert(0);
#endif

}

int netmap_trylock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_trylock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_trylock(&(lock->lock));
#else
    assert(0);
#endif

}

int netmap_unlock(netmap_lock_t *lock)
{
#if defined(NM_USE_MUTEX_LOCK)
    return pthread_mutex_unlock(&(lock->lock));
#elif defined(NM_USE_SPIN_LOCK)
    return pthread_spin_unlock(&(lock->lock));
#else
    assert(0);
#endif
}


