
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include "emi_thread.c"
#include "../catch.hpp"

/*
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
/
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
    atexit(ate);
    emi_thread_pool_destroy(&pool);
    //while(1);
    //
    return 0;
}
*/

bool check_thread(struct emi_thread_pool *pool){

    struct emi_thread *pos;
    list_for_each_entry(pos, &pool->head, list){
        if(pos->status == THREAD_BUSY){
            return false;
        }
    }

    return true;
}

TEST_CASE("emi_thread_pool_init"){
    struct emi_thread_pool pool1;
    int ret = emi_thread_pool_init(&pool1, 3);
    CHECK(ret == 0);
    sleep(1);
    emi_thread_pool_destroy(&pool1);
}


TEST_CASE("emi_thread_pool_submit"){
    using namespace std::chrono_literals;
    using std::this_thread::sleep_for;

    struct emi_thread_pool pool;
    int ret = emi_thread_pool_init(&pool, 3);
    std::atomic<int> counter(0);
    CHECK(ret == 0);

    auto thread_func1 = [](void *args){
        std::atomic<int>& counter = *reinterpret_cast<std::atomic<int> *>(args);
        counter += 1;
    };

    sleep_for(100ms); //waiting for emi_thread_pool_init ready

    SECTION("emi_thread_pool_submit 1"){
        CHECK(emi_thread_pool_submit(&pool, thread_func1, (void *)&counter) == 0);
        CHECK(emi_thread_pool_submit(&pool, thread_func1, (void *)&counter) == 0);
        CHECK(emi_thread_pool_submit(&pool, thread_func1, (void *)&counter) == 0);
        CHECK(emi_thread_pool_submit(&pool, thread_func1, (void *)&counter) == 0);

        sleep_for(100ms);
        CHECK(counter == 4);
        CHECK(check_thread(&pool));
    }

    emi_thread_pool_destroy(&pool);
}


TEST_CASE("emi_thread_pool_destroy"){
    using namespace std::chrono_literals;
    using std::this_thread::sleep_for;

    struct emi_thread_pool pool;
    int ret = emi_thread_pool_init(&pool, 3);

    sleep_for(100ms); //waiting for emi_thread_pool_init ready
    
    CHECK(!list_empty(&pool.head));
    emi_thread_pool_destroy(&pool);
    CHECK(list_empty(&pool.head));
}

