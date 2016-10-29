#ifndef __EMI_SEMAPHORE_H__
#define __EMI_SEMAPHORE_H__

#include <pthread.h>

typedef pthread_mutex_t elock_t;
typedef pthread_spinlock_t espinlock_t;

static int inline emi_spin_init(espinlock_t *p){
    return pthread_spin_init(p,PTHREAD_PROCESS_SHARED);
}

static int inline emi_spin_trylock(espinlock_t *p){
    return pthread_spin_trylock(p);
}

static int inline emi_spin_lock(espinlock_t *p){
    return pthread_spin_lock(p);
}

static int inline emi_spin_unlock(espinlock_t *p){
    return pthread_spin_unlock(p);
}
static int inline emi_spin_destroy(espinlock_t *p){
    return pthread_spin_destroy(p);
}

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
