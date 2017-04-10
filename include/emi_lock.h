#ifndef __EMI_LOCK_H__
#define __EMI_LOCK_H__

#include <pthread.h>

typedef pthread_cond_t econd_t;
typedef pthread_mutex_t elock_t;
typedef pthread_spinlock_t espinlock_t;

inline static int emi_cond_init(econd_t *p){
    return pthread_cond_init(p, NULL);
}

inline static int emi_cond_destroy(econd_t *p){
    return pthread_cond_destroy(p);
}

inline static int emi_cond_wait(econd_t *p, elock_t *l){
    return pthread_cond_wait(p, l);
}

inline static int emi_cond_signal(econd_t *p){
    return pthread_cond_signal(p);
}

inline static int emi_cond_broadcast(econd_t *p){
    return pthread_cond_broadcast(p);
}

inline static int emi_spin_init(espinlock_t *p){
    return pthread_spin_init(p,PTHREAD_PROCESS_SHARED);
}

inline static int emi_spin_trylock(espinlock_t *p){
    return pthread_spin_trylock(p);
}

inline static int emi_spin_lock(espinlock_t *p){
    return pthread_spin_lock(p);
}

inline static int emi_spin_unlock(espinlock_t *p){
    return pthread_spin_unlock(p);
}
inline static int emi_spin_destroy(espinlock_t *p){
    return pthread_spin_destroy(p);
}

inline static int emi_lock_init(elock_t *p){
    return pthread_mutex_init(p,NULL);
}

inline static int emi_trylock(elock_t *p){
    return pthread_mutex_trylock(p);
}

inline static int emi_lock(elock_t *p){
    return pthread_mutex_lock(p);
}

inline static int emi_unlock(elock_t *p){
    return pthread_mutex_unlock(p);
}
inline static int emi_lock_destroy(elock_t *p){
    return pthread_mutex_destroy(p);
}

#endif
