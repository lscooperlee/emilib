#ifndef __EMI_LOCK_H__
#define __EMI_LOCK_H__

#include <pthread.h>

typedef pthread_cond_t econd_t;
typedef pthread_mutex_t emutex_t;
typedef pthread_spinlock_t espinlock_t;
typedef pthread_rwlock_t erwlock_t;

inline static int emi_cond_init(econd_t *p){
    return pthread_cond_init(p, NULL);
}

inline static int emi_cond_destroy(econd_t *p){
    return pthread_cond_destroy(p);
}

inline static int emi_cond_wait(econd_t *p, emutex_t *l){
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

inline static int emi_mutex_init(emutex_t *p){
    return pthread_mutex_init(p,NULL);
}

inline static int emi_mutex_trylock(emutex_t *p){
    return pthread_mutex_trylock(p);
}

inline static int emi_mutex_lock(emutex_t *p){
    return pthread_mutex_lock(p);
}

inline static int emi_mutex_unlock(emutex_t *p){
    return pthread_mutex_unlock(p);
}
inline static int emi_mutex_destroy(emutex_t *p){
    return pthread_mutex_destroy(p);
}

inline static int emi_rwlock_init(erwlock_t *p){
    return pthread_rwlock_init(p, NULL);
}

inline static int emi_rwlock_rdlock(erwlock_t *p){
    return pthread_rwlock_rdlock(p);
}

inline static int emi_rwlock_wrlock(erwlock_t *p){
    return pthread_rwlock_wrlock(p);
}

inline static int emi_rwlock_unlock(erwlock_t *p){
    return pthread_rwlock_unlock(p);
}

inline static int emi_rwlock_destroy(erwlock_t *p){
    return pthread_rwlock_destroy(p);
}

#endif
