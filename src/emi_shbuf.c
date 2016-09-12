/*
EMI:    embedded message interface
Copyright (C) 2009  Cooper <davidontech@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "emi_shbuf.h"
#include "emi_semaphore.h"
#include "emi.h"


elock_t __emi_msg_space_lock;
elock_t __emi_data_space_lock;

elock_t msg_map_lock;
elock_t critical_shmem_lock;

void emi_init_locks(void){
    emi_lock_init(&__emi_msg_space_lock);
    emi_lock_init(&msg_map_lock);
    emi_lock_init(&critical_shmem_lock);
};
/// new 

void *emi_shmbuf_base_addr = NULL;
struct emi_buf *emi_buf_vector = NULL;

#define BUDDY_IDX(buf, top)   ((buf) - (top))
#define PARENT(buf, top)    (((BUDDY_IDX(buf, top) + 1) >> 1) + top - 1)
#define SON(buf, top)    ((BUDDY_IDX(buf, top) << 1) + top + 1)
#define DAUGHTER(buf, top)    ((BUDDY_IDX(buf, top) << 1) + top + 2)
#define SIBLING(buf, top)   (PARENT(buf, top) == PARENT(buf + 1, top) ? buf + 1 : buf - 1)
#define LEFT_MOST_OFFSPRING(level, top)    ((1<<level) + top - 1)

static inline struct emi_buf *get_emi_buf(struct emi_buf *top, int order, int order_num){
    struct emi_buf *tmp = top;
    if(tmp->order < order)
        return NULL;

    int level = order_num;
    while(--level > order){
        struct emi_buf *son = SON(tmp, top);
        struct emi_buf *daughter = DAUGHTER(tmp, top);
        struct emi_buf *next;

        if(son->order < order){
           next = daughter;
        }else if(daughter->order < order){
           next = son;
        }else{
            next = son->order <= daughter->order ? son : daughter;
        }

        tmp = next;
    }

    return tmp;
}

static void update_buf_order(struct emi_buf *buf, struct emi_buf *top, int order, int order_num){

    while(order++ < order_num - 1){
        struct emi_buf *sibling = SIBLING(buf, top);
        struct emi_buf *parent = PARENT(buf, top);

        if(sibling->order < 0 && buf->order < 0){
            parent->order = -order;
        }else if(sibling->order == buf->order){
            parent->order++;
        }else{
            parent->order = sibling->order > buf->order ? sibling->order : buf->order;
        }
        buf = parent;
    }
}

static int __init_emi_buf(struct emi_buf *top, void *addr, int order){
    int i;
    for (i = 0; i < order; i++)
    {
        int j;
        int current_order = 1<<i;
        struct emi_buf *tmp = LEFT_MOST_OFFSPRING(i, top);
        for (j = 0; j < current_order; j++, tmp++)
        {
            tmp->addr = addr + j * (order - i) * BUDDY_SIZE;
            tmp->order = order - i - 1;
        }
    }

    return 0;
}

static struct emi_buf *__alloc_emi_buf(struct emi_buf *top, int order, int order_num){
    struct emi_buf *tmp = get_emi_buf(top, order, order_num);

    if(tmp == NULL)
        return NULL;

    tmp->order = -order - 1;

    update_buf_order(tmp, top, order, order_num);

    return tmp;
}

static void __free_emi_buddy(struct emi_buf *buddy, struct emi_buf *top, int order, int order_num){

    buddy->order = order;

    update_buf_order(buddy, top, order, order_num);
}

int init_emi_buf(void *base){
    struct emi_buf *top = (struct emi_buf *)malloc(((1<<EMI_ORDER_NUM) - 1) * BUDDY_SIZE);
    if(top == NULL)
        return -1;

    emi_shmbuf_base_addr = base;
    emi_buf_vector = top;

    return __init_emi_buf(top, base, EMI_ORDER_NUM);
}

struct emi_buf *alloc_emi_buf(int order){
    //size->order

    return __alloc_emi_buf(emi_buf_vector, order, EMI_ORDER_NUM);
}

void free_emi_buf(struct emi_buf *buf){
    int order = -buf->order - 1;

    __free_emi_buddy(buf, emi_buf_vector, order, EMI_ORDER_NUM);
}

