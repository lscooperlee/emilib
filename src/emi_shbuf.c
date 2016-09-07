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


LIST_HEAD(__msg_list);
LIST_HEAD(__data_list);
eu32 __num_busy_data=0;
eu32 __num_busy_msg=0;

elock_t __emi_msg_space_lock;
elock_t __emi_data_space_lock;

elock_t msg_map_lock;
elock_t critical_shmem_lock;

#ifdef DEBUG
void debug_emi_transfer_buf(struct emi_transfer_buf *buf){
    printf("debug_emi_transfer_buf:\n    ");
    printf("buf->offset=%d,buf->busy=%d,buf->addr=%p\n",buf->offset,buf->busy,buf->addr);
}
#endif

int emi_init_space(struct list_head *head,eu32 num,es32 bias_base,eu32 size){
    int i;
    struct emi_transfer_buf *buf;
    struct list_head *lh;
    for(i=1;i<=num;i++){
        if((buf=(struct emi_transfer_buf *)malloc(sizeof(struct emi_transfer_buf)))==NULL){
            goto e1;
        }
        list_add_tail(&buf->list,head);
        buf->offset=bias_base+i*size;
        buf->busy=SPACE_FREE;
        buf->addr=NULL;
    }
    return 0;
e1:
    list_for_each(lh,head){
        buf=container_of(lh,struct emi_transfer_buf,list);
        list_del(&buf->list);
        free(buf);
    }
    return -1;
}

/*
 *
 * three locks need to be initialized:
 * __emi_msg_space_lock:when allocing emi_msg space to store a recieved emi_msg, we should use the lock-->alloc-->unlock sequence.
 * msg_map_lock:this lock is used for emi_core to search hash table to find the right process.
 * critical_shmem_lock:the critical_shmem_lock is used for msg space index area ,the index area contains one sizeof(int) length of index for each process ,the total size of which is sizeof(int)*(system max process number).when one process recieved more than one massages,the only index can not point to two emi_msg space,thus critical_shmem_lock is needed.
 *
 *
 * */
void emi_init_locks(void){
    emi_lock_init(&__emi_msg_space_lock);
    emi_lock_init(&msg_map_lock);
    emi_lock_init(&critical_shmem_lock);
};


struct emi_buddy{
    struct list_head list;
    void *addr;
};

#ifndef MAX_ORDER_NUM
#define MAX_ORDER_NUM 8
#endif

#define BUDDY_SIZE  ((sizeof(struct emi_msg) + 0xFF) & ~0xFF)

static struct emi_buddy *emi_buddy_allocator = NULL;
static struct list_head *free_memlist_vector = NULL;

#define BUDDY_IDX(buddy_addr)   (buddy_addr - emi_buddy_allocator)

#define PARENT(buddy_addr)    ((BUDDY_IDX(buddy_addr) >> 1) + emi_buddy_allocator)
#define SON(buddy_addr)    ((BUDDY_IDX(buddy_addr) << 1) + emi_buddy_allocator + 1)
#define DAUGHTER(buddy_addr)    ((BUDDY_IDX(buddy_addr) << 1) + emi_buddy_allocator + 2)

static struct emi_buddy *inherit_from_parent(int myorder){
    int parent_order = myorder + 1;

    struct list_head *parent = &free_memlist_vector[parent_order];

    if (parent == NULL)
        return NULL;

    struct emi_buddy *buddy = list_first_entry(parent, struct emi_buddy, list);
    struct emi_buddy *daughter = DAUGHTER(buddy);
    struct emi_buddy *son = SON(buddy);

    buddy->addr = NULL;

    list_add(&daughter->list, &free_memlist_vector[myorder]);

    return son;
}

static struct emi_buddy *pass_to_child(struct emi_buddy *buddy, int myorder){
    int child_order = myorder - 1;

    struct emi_buddy *daughter = DAUGHTER(buddy);
    struct emi_buddy *son = SON(buddy);

    buddy->addr = NULL;

    list_add(&daughter->list, &free_memlist_vector[child_order]);

    return son;

}

static void *inherit_from_ancestor(int order){
    int my_order = order;
    struct emi_buddy *addr = NULL;

    while(order < MAX_ORDER_NUM - 1){
        addr = inherit_from_parent(order++);
        if(addr != NULL){
            break;
        }
    }
    
    if(addr == NULL)
        return NULL;

    while(order > my_order + 1){
        pass_to_child(addr, --order);
    }

    return addr;
}

static void init_emi_buddy(struct emi_buddy *buddy, void *addr){
    INIT_LIST_HEAD(&buddy->list);
    buddy->addr = addr;
    
    int i, j;
    for(i = 0; i < MAX_ORDER_NUM; i++){
        int order = 1<<i;
        
        struct emi_buddy *tmp = buddy;
        for(j = 0; j<order; j++){
            tmp->addr = buddy->addr + j * (MAX_ORDER_NUM - i);    
        }
        buddy = SON(buddy);
    }
}

int emi_init_msgbuf_allocator(void *base_addr){
    
    unsigned int num_buddy = (1<<(MAX_ORDER_NUM+1)) - 1;

    emi_buddy_allocator = (struct emi_buddy *)malloc(sizeof(struct emi_buddy) * num_buddy);
    if(emi_buddy_allocator == NULL){
        return -1;
    }

    init_emi_buddy(&emi_buddy_allocator[0], base_addr);

    free_memlist_vector = (struct list_head*)malloc(sizeof(struct list_head) * MAX_ORDER_NUM);
    if(free_memlist_vector == NULL){
        free(emi_buddy_allocator);
        return -1;
    }
    
    int i;
    for(i = 0; i < MAX_ORDER_NUM - 1; i++){
        INIT_LIST_HEAD(&free_memlist_vector[i]);
    }
    list_add(&emi_buddy_allocator[0].list, &free_memlist_vector[MAX_ORDER_NUM - 1]); 

    return 0;
}

void *__emi_msgbuf_alloc(unsigned int order){
    struct emi_buddy *buddy = NULL;
    
    if (!list_empty(&free_memlist_vector[order])){
        buddy = list_first_entry(&free_memlist_vector[order], struct emi_buddy, list);
        list_del_first_entry(&free_memlist_vector[order]);
    }else{
        buddy = inherit_from_ancestor(order);
    }
    
    if(buddy != NULL){
        void *addr = buddy->addr;
        buddy->addr = NULL;
        return addr;
    }

    return NULL;
}

void __emi_msgbuf_free(void *addr){
    
}

