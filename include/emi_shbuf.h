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


#define emi_msg_shbuf_alloc()             (struct emi_msg *)emi_alloc(sizeof(struct emi_msg))
#define emi_msg_shbuf_free(addr)          emi_free(addr)
#define emi_msg_shbuf_realloc(emi_msg)    emi_msg_realloc_for_data(emi_msg)

#define emi_get_space_msg_num(base,addr) __get_space_num(base,addr,1)
static inline int __get_space_num(void *base,void *addr,int size){
    return ((char *)addr-(char *)base)/size;
}


#ifndef EMI_ORDER_NUM
#define EMI_ORDER_NUM 10
#endif

#define BUDDY_SHIFT 8
#define BUDDY_SIZE  (1<<BUDDY_SHIFT)
//#define BUDDY_SIZE  ((sizeof(struct emi_msg) + EMI_MAX_MSG_SIZE) & ~EMI_MAX_MSG_SIZE)

struct emi_buf{
    void *addr;
    int order;
};

extern int init_emi_buf(void *base);

extern struct emi_buf *alloc_emi_buf(size_t size);

extern void free_emi_buf(struct emi_buf *buf);

extern void *emi_alloc(size_t size);
    
extern void emi_free(void *addr);

extern struct emi_msg *emi_msg_realloc_for_data(struct emi_msg *msg);

#endif
