
#include <stdio.h>
#include <unistd.h>

#include "emi_thread.h"
#include "emi_dbg.h"

void func(void *param){
    emi_printf("func \n");
    sleep(1);
    emi_printf("after func \n");
}


struct emi_thread_pool pool;

void ate(void){
    perror("xxx");
    emi_thread_pool_destroy(&pool);
}

int main(){
    int ret = emi_thread_pool_init(&pool, 5);
    emi_printf("%d\n", ret);
    sleep(1);
    
    emi_thread_pool_submit(&pool, func, NULL);
    emi_thread_pool_submit(&pool, func, NULL);
    emi_thread_pool_submit(&pool, func, NULL);
    emi_thread_pool_submit(&pool, func, NULL);
    emi_thread_pool_submit(&pool, func, NULL);
    emi_thread_pool_submit(&pool, func, NULL);

    emi_printf("%d\n", ret);

    //sleep(3);
    //atexit(ate);
    emi_thread_pool_destroy(&pool);
    //while(1);
}
