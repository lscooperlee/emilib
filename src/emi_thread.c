
#include <stdlib.h>
#include <signal.h>
#include <list.h>
#include <assert.h>

#include "emi_thread.h"
#include "emi_lock.h"
#include "emi_dbg.h"

static void *thread_func(void *t){
    assert(t != NULL);

    struct emi_thread *thread = (struct emi_thread *)t;
    struct emi_thread_pool *pool = thread->pool;

    emi_lock(&thread->lock);
    while(1){
        emi_cond_wait(&thread->cond, &thread->lock);

        //int state = PTHREAD_CANCEL_DISABLE;
        //pthread_setcancelstate(thread->thread, &state);
        //
        if(thread->func != NULL){
            thread->func(thread->args);
        }

        //state = PTHREAD_CANCEL_ENABLE;
        //pthread_setcancelstate(thread->thread, &state);

        thread->status = THREAD_IDLE;

        emi_spin_lock(&pool->spinlock);
        list_move(&thread->list, &pool->head);
        emi_spin_unlock(&pool->spinlock);

    };

    emi_unlock(&thread->lock);
    return NULL;    
}

int emi_thread_init(struct emi_thread *th, struct emi_thread_pool *pool){
    if(emi_cond_init(&th->cond)){
        goto out;
    }

    if(emi_lock_init(&th->lock)){
        goto out_cond;
    }

    INIT_LIST_HEAD(&th->list);
    th->status = THREAD_IDLE;
    th->pool = pool;

    if(pthread_create(&th->thread, NULL, thread_func, (void *)th)){
        goto out_lock;
    }

    return 0;

out_lock:
    emi_lock_destroy(&th->lock);
out_cond:
    emi_cond_destroy(&th->cond);
out:
    return -1;
}

void emi_thread_destroy(struct emi_thread *th){
    list_del(&th->list);

    pthread_cancel(th->thread);

    emi_unlock(&th->lock);
    emi_lock_destroy(&th->lock);
    emi_cond_destroy(&th->cond);
}

static struct emi_thread *emi_thread_create(struct emi_thread_pool *pool){
    struct emi_thread *th = (struct emi_thread *)malloc(sizeof(struct emi_thread));
    
    if(th == NULL){
        return NULL;
    }

    if(emi_thread_init(th, pool)){
        free(th);
        return NULL;
    }
    
    return th;
}

void emi_thread_free(struct emi_thread *th){
    emi_thread_destroy(th);
    pthread_join(th->thread, NULL);
    free(th);
}

void emi_thread_pool_destroy(struct emi_thread_pool *pool){
    
    emi_spin_lock(&pool->spinlock);
    struct list_head *pos, *n;    
    list_for_each_safe(pos, n, &pool->head){
        struct emi_thread *t = list_entry(pos, struct emi_thread, list);
        emi_thread_free(t); 
    }
    emi_spin_unlock(&pool->spinlock);

    emilog(EMI_DEBUG, "thread pool destroy\n");

    emi_spin_destroy(&pool->spinlock);
}

int emi_thread_pool_init(struct emi_thread_pool *pool, size_t size){

    if(emi_spin_init(&pool->spinlock)){
        return -1;
    }

    INIT_LIST_HEAD(&pool->head);
    pool->pool_size = size;

    while(size--){
        struct emi_thread *t = emi_thread_create(pool);
        if(t == NULL){
            goto free;               
        }

        list_add(&t->list, &pool->head);
    }

    emilog(EMI_DEBUG, "thread pool init with size %u\n", pool->pool_size);
    
    return 0;

free:

    emi_thread_pool_destroy(pool);
    return -1;
}

int emi_thread_pool_submit(struct emi_thread_pool *pool, emi_thread_func func, void *args){

    emi_spin_lock(&pool->spinlock);
    
    struct emi_thread *t = list_first_entry(&pool->head, struct emi_thread, list);

    //Don't need lock for t->status
    if(t->status == THREAD_IDLE){
        t->status = THREAD_BUSY;
        
        t->func = func;
        t->args = args;

        list_move_tail(&t->list, &pool->head);

        emi_spin_unlock(&pool->spinlock);

        emi_cond_signal(&t->cond);

        emilog(EMI_DEBUG, "thread pool submit \n");

    }else{
        emi_spin_unlock(&pool->spinlock);
        
        pthread_t th;
        if(pthread_create(&th, NULL, (void *(*)(void *))func, args)){
            return -1;
        }
        pthread_detach(th);
        
        emilog(EMI_DEBUG, "new thread create \n");
    }


    return 0;
}
