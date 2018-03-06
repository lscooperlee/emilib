#ifndef __EMI_THREAD_H__
#define __EMI_THREAD_H__

#include <pthread.h>

#include "emi_lock.h"
#include "emi_types.h"
#include "list.h"

enum {
    THREAD_BUSY,
    THREAD_IDLE,
};

typedef void (*emi_thread_func)(void *);

struct emi_thread {
    struct list_head list;
    pthread_t thread;
    elock_t lock;
    econd_t cond;
    int status;
    emi_thread_func func;
    void *args;
    struct emi_thread_pool *pool;
};

struct emi_thread_pool {
    espinlock_t spinlock;
    eu32 pool_size;
    eu32 threads_num;
    struct list_head head;
};

extern int emi_thread_pool_init(struct emi_thread_pool *pool, size_t size);

extern void emi_thread_pool_destroy(struct emi_thread_pool *pool);

extern int emi_thread_pool_submit(struct emi_thread_pool *pool, emi_thread_func func, void *args);

#endif
