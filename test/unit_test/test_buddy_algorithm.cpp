
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <random>


#define EMI_ORDER_NUM 4

#include "emi_shbuf.h"

#include "emi_shbuf.c"

#include "../catch.hpp"

constexpr int get_reverse_order(int s){
    int size = s+1;
    int order = 0;
    while(size > 0){
        size >>= 1;
        order++;
    }
    return order;
}

constexpr int get_order_from_idx(int s){
    return EMI_ORDER_NUM - get_reverse_order(s);
}
constexpr int get_blkoffset_from_idx(int s){
    int span = 1 << (EMI_ORDER_NUM - 1);
    int rorder = get_reverse_order(s);
    int num_in_level = 1 << (rorder - 1);
    
    return (span/num_in_level) * (s - num_in_level + 1);
}

void print_emi_buf(struct emi_buf *mgr){
    int k;
    int t=0;
    for(k=0;k<(1<<EMI_ORDER_NUM)-1; k++){
        struct emi_buf *tmp = &mgr[k];
        printf("ofst:%d,odr:%d  ", tmp->blk_offset, tmp->order);

        if(t==0){
            printf("\n");
            t=1<<get_reverse_order(k);
        }
            t--;
    }
    printf("\n\n");
}

constexpr int get_buf_size(){
    return (1<<EMI_ORDER_NUM) - 1;
}

bool check_emi_buf(struct emi_buf *mgr, const std::unordered_map<int, int>& map = {}){
    for(int i=0; i<get_buf_size(); ++i){
        const auto& tmp = mgr[i];
        auto order = get_order_from_idx(i);

        if(map.find(i) != map.end()){
            if(map.at(i) != tmp.order){
                return false;
            }
        }else{
            if(tmp.order != order){
                return false;
            }
        }
    }

    return true;
}

void test_example(){

    void *base_addr = malloc(BUDDY_SIZE << EMI_ORDER_NUM);
    struct emi_buf array[(1<<EMI_ORDER_NUM) - 1] = {};
    struct emi_buf *a,*b,*c,*d;

    init_emi_buf(base_addr, array);
    print_emi_buf(array);

    a = alloc_emi_buf(BUDDY_SIZE>>1);
    print_emi_buf(array);

    b = alloc_emi_buf(BUDDY_SIZE/2 + BUDDY_SIZE);
    print_emi_buf(array);

    c = alloc_emi_buf(BUDDY_SIZE/2);
    print_emi_buf(array);

    d = alloc_emi_buf(BUDDY_SIZE/2 + BUDDY_SIZE);
    print_emi_buf(array);

    free_emi_buf(b);
    print_emi_buf(array);
    
    free_emi_buf(d);
    print_emi_buf(array);

    free_emi_buf(a);
    print_emi_buf(array);

    free_emi_buf(c);
    print_emi_buf(array);

}

TEST_CASE("__init_emi_buf") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};

    __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);
    CHECK(check_emi_buf(emi_buf_top));
}

TEST_CASE("get_emi_buf") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};

    __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);

    SECTION("request order larger than maximum order") { // maximum order = EMI_ORDER_NUM - 1
        auto buf = get_emi_buf(emi_buf_top, 4, EMI_ORDER_NUM);
        CHECK(buf == nullptr);
    }

    SECTION("request order equal to maximum order") {
        auto buf = get_emi_buf(emi_buf_top, 3, EMI_ORDER_NUM); 
        CHECK(buf == emi_buf_top);
    }

    SECTION("request order 2") {
        auto buf = get_emi_buf(emi_buf_top, 2, EMI_ORDER_NUM); 
        CHECK(buf == emi_buf_top + 1);
    }

    SECTION("request order 1") {
        auto buf = get_emi_buf(emi_buf_top, 1, EMI_ORDER_NUM); 
        CHECK(buf == emi_buf_top + 3);
    }

    SECTION("request order 0") {
        auto buf = get_emi_buf(emi_buf_top, 0, EMI_ORDER_NUM); 
        CHECK(buf == emi_buf_top + 7);
    }
}

TEST_CASE("update_buf_order") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};
    __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);

    SECTION("update emi buf top") {

        struct emi_buf *buf = emi_buf_top;
        buf->order = -4;
        update_buf_order(buf, emi_buf_top, 3, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, -4}}));
    }

    SECTION("update emi buf 1") {

        struct emi_buf *buf = emi_buf_top + 1;
        buf->order = -3;
        update_buf_order(buf, emi_buf_top, 2, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, 2}, {1, -3}}));
    }

    SECTION("update emi buf 6") {

        struct emi_buf *buf = emi_buf_top + 6;
        buf->order = -2;
        update_buf_order(buf, emi_buf_top, 1, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, 2}, {2, 1}, {6, -2}}));
    }
}

TEST_CASE("__alloc_emi_buf") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};
    __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);

    SECTION("alloc order 3") {

        auto buf = __alloc_emi_buf(emi_buf_top, 3, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, -4}}));
        CHECK(buf == emi_buf_top);
    }

    SECTION("alloc order 2") {

        auto buf = __alloc_emi_buf(emi_buf_top, 2, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, 2}, {1, -3}}));
        CHECK(buf == emi_buf_top + 1);
    }

    SECTION("alloc order 1") {

        auto buf = __alloc_emi_buf(emi_buf_top, 1, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top, {{0, 2}, {1, 1}, {3, -2}}));
        CHECK(buf == emi_buf_top + 3);
    }
}

TEST_CASE("__free_emi_buf") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};
    __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);

    SECTION("free order 3") {

        auto buf = __alloc_emi_buf(emi_buf_top, 3, EMI_ORDER_NUM);
        __free_emi_buf(buf, emi_buf_top, 3, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top));
    }

    SECTION("free order 2") {

        auto buf = __alloc_emi_buf(emi_buf_top, 2, EMI_ORDER_NUM);
        __free_emi_buf(buf, emi_buf_top, 2, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top));
    }

    SECTION("free order 1") {

        auto buf = __alloc_emi_buf(emi_buf_top, 1, EMI_ORDER_NUM);
        __free_emi_buf(buf, emi_buf_top, 1, EMI_ORDER_NUM);
        CHECK(check_emi_buf(emi_buf_top));
    }

    SECTION("free order random") {

        std::vector<int> orders = {0, 0, 0, 1, 1, 1, 2, 2};
        std::random_device rd;
        std::mt19937 g(rd());
        shuffle(orders.begin(), orders.end(), g);

        std::vector<struct emi_buf *> bufs;

        for(const auto order: orders){
            struct emi_buf *buf = __alloc_emi_buf(emi_buf_top, order, EMI_ORDER_NUM);
            bufs.push_back(buf);
        }

        for(unsigned int i = 0; i<orders.size(); ++i){
            if(bufs[i] != NULL) {
                __free_emi_buf(bufs[i], emi_buf_top, orders[i], EMI_ORDER_NUM);
            }
        }
    
        CHECK(check_emi_buf(emi_buf_top));
    }
}

TEST_CASE("size_to_order") {
    CHECK(size_to_order(BUDDY_SIZE - 1) == 0);
    CHECK(size_to_order(BUDDY_SIZE + 1) == 1);
    CHECK(size_to_order(2 * BUDDY_SIZE) == 1);
    CHECK(size_to_order(3 * BUDDY_SIZE) == 2);
    CHECK(size_to_order(4 * BUDDY_SIZE + 1) == 3);
}

TEST_CASE("emi_alloc") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};
    void *base = malloc(BUDDY_SIZE * 1<<(BUDDY_SHIFT - 1));
    espinlock_t buf_lock;

    int ret = init_emi_buf_lock(base, emi_buf_top, &buf_lock);
    CHECK(ret == 0);
    CHECK(check_emi_buf(emi_buf_top));

    SECTION("emi_alloc extreme value") {
        void *mem;

        mem = emi_alloc(0);
        CHECK(mem == NULL);

        mem = emi_alloc(BUDDY_SIZE << BUDDY_SHIFT);
        CHECK(mem == NULL);
    }

    SECTION("emi_alloc 1 block") {
        void *mem = emi_alloc(BUDDY_SIZE - 37);

        struct emi_buf *buf = GET_BUF_ADDR(mem) + emi_buf_top;
        
        CHECK((char *)base == ((char *)(mem) - 4));
        CHECK(buf == &emi_buf_top[7]);
    }

    SECTION("emi_alloc 2 block") {
        void *mem;
        mem = emi_alloc(BUDDY_SIZE - 37);
        mem = emi_alloc(BUDDY_SIZE + BUDDY_SIZE - 43);

        struct emi_buf *buf = GET_BUF_ADDR(mem) + emi_buf_top;
        
        CHECK(((char *)base + BUDDY_SIZE*2) == ((char *)(mem) - 4));
        CHECK(buf == &emi_buf_top[4]);
    }
}

TEST_CASE("emi_free") {
    struct emi_buf emi_buf_top[get_buf_size()] = {};
    void *base = malloc(BUDDY_SIZE * 1<<(BUDDY_SHIFT - 1));
    espinlock_t buf_lock;

    int ret = init_emi_buf_lock(base, emi_buf_top, &buf_lock);
    CHECK(ret == 0);
    CHECK(check_emi_buf(emi_buf_top));

    SECTION("emi_random free") {

        std::vector<int> sizes = {47, 192, 543, 1984, 675, 9812, 65521};
        std::random_device rd;
        std::mt19937 g(rd());
        shuffle(sizes.begin(), sizes.end(), g);

        std::vector<void *> mems;

        for(const auto size: sizes){
            void *mem = emi_alloc(size);
            mems.push_back(mem);
        }

        for(unsigned int i = 0; i<sizes.size(); ++i){
            if(mems[i] != NULL) {
                emi_free(mems[i]);
            }
        }

        CHECK(check_emi_buf(emi_buf_top));
    }
}
