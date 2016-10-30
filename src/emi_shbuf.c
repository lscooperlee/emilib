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


static void *emi_shmbuf_base_addr = NULL;
static struct emi_buf *emi_buf_vector = NULL;

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
            parent->order = -order-1;
        }else if(sibling->order == buf->order){
            parent->order++;
        }else{
            parent->order = sibling->order > buf->order ? sibling->order : buf->order;
        }
        buf = parent;
    }
}

static int __init_emi_buf(struct emi_buf *top, int order){
    int i;
    for (i = 0; i < order; i++)
    {
        int j;
        int current_order = 1<<i;
        struct emi_buf *tmp = LEFT_MOST_OFFSPRING(i, top);
        for (j = 0; j < current_order; j++, tmp++)
        {
            tmp->blk_offset = j * (order - i);
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

int init_emi_buf(void *base, void *emi_buf_top){

    emi_shmbuf_base_addr = base;
    emi_buf_vector = emi_buf_top;

    return __init_emi_buf(emi_buf_top, EMI_ORDER_NUM);
}

struct emi_buf *alloc_emi_buf(size_t size){
    if(size == 0)
        return NULL;

    int order = 0;
    size = (size-1) >> BUDDY_SHIFT;

    while(size > 0){
        size >>= 1;
        order++;
    }

    return __alloc_emi_buf(emi_buf_vector, order, EMI_ORDER_NUM);
}

void free_emi_buf(struct emi_buf *buf){
    int order = -buf->order - 1;

    __free_emi_buddy(buf, emi_buf_vector, order, EMI_ORDER_NUM);
}

#define ADDR_BUF_OFFSET  (sizeof(struct emi_buf *))
#define GET_ALLOC_ADDR(buf)    (((buf)->blk_offset << BUDDY_SHIFT) + emi_shmbuf_base_addr + ADDR_BUF_OFFSET)
#define GET_BUF_ADDR(addr)    *(struct emi_buf **)(((void *)addr) - ADDR_BUF_OFFSET)

static espinlock_t *emi_buf_lock = NULL;

int init_emi_buf_lock(void *base, void *emi_buf_top, espinlock_t *lock){
    init_emi_buf(base, emi_buf_top);
    emi_buf_lock = lock;
    return emi_spin_init(emi_buf_lock);
}

void *emi_alloc(size_t size){
    emi_spin_lock(emi_buf_lock);
    struct emi_buf *buf = alloc_emi_buf(size + ADDR_BUF_OFFSET);
    emi_spin_unlock(emi_buf_lock);
    
    void *addr = GET_ALLOC_ADDR(buf);
    
    GET_BUF_ADDR(addr) = buf;

    return addr;
}

void emi_free(void *addr){
    struct emi_buf *buf = GET_BUF_ADDR(addr);

    emi_spin_lock(emi_buf_lock);
    free_emi_buf(buf);
    emi_spin_unlock(emi_buf_lock);
}

struct emi_msg *emi_realloc_for_data(struct emi_msg *msg){

    int newsize =msg->size;
    
    struct emi_buf *buf = GET_BUF_ADDR(msg);
    int order = -buf->order - 1;

    int oldsize = (1<<order) << BUDDY_SHIFT;

    if(oldsize >= newsize){
        return msg;
    }

    struct emi_msg *newaddr = (struct emi_msg *)emi_alloc(newsize);

    memcpy(newaddr, msg, sizeof(struct emi_msg));

    emi_free(msg);

    return newaddr;
}
