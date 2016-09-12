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

#ifndef __SHBUF_H__
#define __SHBUF_H__

#include "list.h"
#include "emi_semaphore.h"
#include "emi_types.h"
#include "emi.h"
#include "emi_config.h"

extern elock_t __emi_msg_space_lock;
extern elock_t msg_map_lock;
extern elock_t critical_shmem_lock;

#define emi_init_msg_space(base)    init_emi_buf(base)
#define emi_get_msg_space(order)    alloc_emi_buf(0)
#define emi_return_msg_space(buf)   free_emi_buf(buf) 

#define emi_get_space_msg_num(base,addr) __get_space_num(base,addr,1)
static inline int __get_space_num(void *base,void *addr,int size){
    return ((char *)addr-(char *)base)/size;
}



extern void emi_init_locks(void);

#ifndef EMI_ORDER_NUM
#define EMI_ORDER_NUM 10
#endif

#define BUDDY_SIZE  ((sizeof(struct emi_msg) + EMI_MAX_MSG_SIZE) & ~EMI_MAX_MSG_SIZE)

struct emi_buf{
    void *addr;
    int order;
};

extern int init_emi_buf(void *base);

extern struct emi_buf *alloc_emi_buf(int order);

extern void free_emi_buf(struct emi_buf *buf);


#endif
