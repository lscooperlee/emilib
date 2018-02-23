
#include <stdlib.h>
#include <stdio.h>

#undef DEBUG

#define EMI_ORDER_NUM 4

#include "emi_shbuf.h"

#include "emi_shbuf.c"
#include "emi_dbg.c"

//#define CATCH_CONFIG_MAIN
//#include "../catch.hpp"

int get_order(int s){
    int size = s+1;
    int order = 0;
    while(size > 0){
        size >>= 1;
        order++;
    }
    return order;
}

void print_emi_buf(struct emi_buf *mgr){
    int k;
    int t=0;
    for(k=0;k<(1<<EMI_ORDER_NUM)-1; k++){
        struct emi_buf *tmp = &mgr[k];
        printf("ofst:%d,odr:%d  ", tmp->blk_offset, tmp->order);

        if(t==0){
            printf("\n");
            t=1<<get_order(k);
        }
            t--;
    }
    printf("\n\n");
}

void test_example(){

    void *base_addr = malloc(BUDDY_SIZE << EMI_ORDER_NUM);
    struct emi_buf array[(1<<EMI_ORDER_NUM) - 1] = {0};
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

int main(){
}
