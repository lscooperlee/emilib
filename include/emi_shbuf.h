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


#ifndef EMI_ORDER_NUM
#define EMI_ORDER_NUM 10
#endif

#define BUDDY_SHIFT 8
#define BUDDY_SIZE  (1<<BUDDY_SHIFT)
//#define BUDDY_SIZE  ((sizeof(struct emi_msg) + EMI_MAX_MSG_SIZE) & ~EMI_MAX_MSG_SIZE)

struct emi_buf{
    int blk_offset;
    int order;
};

extern int init_emi_buf(void *base, void *emi_buf_top);
extern struct emi_buf *alloc_emi_buf(size_t size);
extern void free_emi_buf(struct emi_buf *buf);

extern int init_emi_buf_lock(void *base, void *emi_buf_top, espinlock_t *lock);
extern void *emi_alloc(size_t size);
extern void emi_free(void *addr);

extern struct emi_msg *alloc_shared_msg(eu32 size);
extern struct emi_msg *realloc_shared_msg(struct emi_msg *msg);
extern void free_shared_msg_data(struct emi_msg *msg);
extern void free_shared_msg(struct emi_msg *msg);

extern void put_msg_data_addr(struct emi_msg *msg);
extern void put_msg_data_offset(struct emi_msg *msg);

extern void update_emi_buf_lock(void *base, void *emi_buf_top, espinlock_t *lock);

#define get_shbuf_offset(base, addr) ((char *)(addr)-(char *)(base))
#define get_shbuf_addr(base, offset) ((void*)base + offset)


#endif
