#include <stdlib.h>
#include "emi_shbuf.h"

/*
void print_emi_buf(struct emi_buf *mgr){
    int i;
    for(i=0;i<ORDER_NUM;i++){
        int j;
        int current_order = 1<<i;
        struct emi_buf *tmp = LEFT_MOST_OFFSPRING(i, mgr);
        printf("%d ", ORDER_NUM - i - 1);
        for (j = 0; j < current_order; j++, tmp++)
        {
            printf("addr:%lx,order:%d  ", (long)tmp->addr&0xFF, tmp->order);
        }
        printf("\n");
    }
    printf("\n");
}
*/

int main(){
/*
    int base_addr[1<<(ORDER_NUM - 1)] = {0};
    struct emi_buf array[(1<<ORDER_NUM) - 1] = {0};
    struct emi_buf *a,*b,*c,*d;

    __init_emi_buf(array, base_addr, ORDER_NUM);
    print_emi_buf(array);

    a = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);

    b = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);

    c = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);

    d = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);

    __free_emi_buddy(a, array, 0);
    print_emi_buf(array);

    __free_emi_buddy(b, array, 0);
    print_emi_buf(array);

    __free_emi_buddy(c, array, 0);
    print_emi_buf(array);

    __free_emi_buddy(d, array, 0);
    print_emi_buf(array);

    a = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);
    b = __alloc_emi_buddy(array, 1);
    print_emi_buf(array);
    c = __alloc_emi_buddy(array, 0);
    print_emi_buf(array);

    __free_emi_buddy(b, array, 1);
    print_emi_buf(array);

    __free_emi_buddy(c, array, 0);
    print_emi_buf(array);

    __free_emi_buddy(a, array, 0);
    print_emi_buf(array);

    */
}
