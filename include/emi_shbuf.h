
#ifndef __SHBUF_H__
#define __SHBUF_H__

#include "list.h"
#include "emi_lock.h"
#include "emi_types.h"
#include "emi_msg.h"
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

extern int init_emi_buf(void *base, struct emi_buf *emi_buf_top);
extern struct emi_buf *alloc_emi_buf(size_t size);
extern void free_emi_buf(struct emi_buf *buf);

extern int init_emi_buf_lock(void *base, struct emi_buf *emi_buf_top, espinlock_t *lock);
extern void *emi_alloc(size_t size);
extern void emi_free(void *addr);

extern struct emi_msg *alloc_shared_msg(eu32 size);
extern struct emi_msg *realloc_shared_msg(struct emi_msg *msg);

extern void free_shared_msg_data(struct emi_msg *msg);
extern void free_shared_msg(struct emi_msg *msg);

extern void update_emi_buf_lock(void *base, void *emi_buf_top, espinlock_t *lock);

#define get_shbuf_offset(base, addr) GET_OFFSET(base, addr)
#define get_shbuf_addr(base, offset) GET_ADDR(base, offset)

#endif
