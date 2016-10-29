#ifndef __EMI_SEMAPHORE_H__
#define __EMI_SEMAPHORE_H__

#include <pthread.h>

typedef pthread_mutex_t elock_t;
typedef pthread_spinlock_t espinlock_t;

static int inline emi_lock_init(elock_t *p){
    return pthread_mutex_init(p,NULL);
}

static int inline emi_trylock(elock_t *p){
    return pthread_mutex_trylock(p);
}

static int inline emi_lock(elock_t *p){
    return pthread_mutex_lock(p);
}

static int inline emi_unlock(elock_t *p){
    return pthread_mutex_unlock(p);
}
static int inline emi_lock_destroy(elock_t *p){
    return pthread_mutex_destroy(p);
}

#endif
