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

#ifndef __SHMEM_H__
#define __SHMEM_H__

#include "list.h"
#include "emi_semaphore.h"
#include "emi_types.h"


#define EMI_KEY                ((key_t)0x0FE02570)

#define LOCK    "/var/run/emi_core.pid"
#define LOCKBASENAME    "/tmp/emi_core."


struct emi_transfer_buf{
    struct list_head list;
    int busy;
    void *addr;
    eu32 offset;
    eu32 size;
};

enum{
    SPACE_FREE,
    SPACE_BUSY,
};

extern struct list_head __msg_list;
extern eu32 __num_busy_msg;
extern elock_t __emi_msg_space_lock;

extern elock_t msg_map_lock;
extern elock_t critical_shmem_lock;

#define GET_SIZE(type) (sizeof(type)+EMI_DATA_SIZE_PER_MSG)

#define emi_init_msg_space(num,bias) emi_init_space(&__msg_list,num,bias,GET_SIZE(struct emi_msg))


#define emi_obtain_msg_space(base) __emi_obtain_space(base,1,&__msg_list,&__num_busy_msg,&__emi_msg_space_lock)
#define emi_return_msg_space(addr) __emi_return_space(addr,&__msg_list,&__num_busy_msg,&__emi_msg_space_lock)
#define obtain_space_msg_num(base,addr) __obtain_space_num(base,addr,1)


#define emi_data_space_query(null)    (__num_busy_data)



static inline int __obtain_space_num(void *base,void *addr,int size){
    return ((char *)addr-(char *)base)/size;
}

/*
*    at present the msg_data area is managered by list. the whole msg_data area is divided into pieces
* */
static inline void *__emi_obtain_space(void *base,int size,struct list_head *head,eu32 *num,elock_t *lock){
    struct emi_transfer_buf *buf;
    buf=container_of(head->next,struct emi_transfer_buf,list);
    if(buf->busy)
        return NULL;
    emi_lock(lock);
    list_move_tail(&buf->list,head);
    buf->busy=SPACE_BUSY;
    emi_unlock(lock);
    buf->addr=(void *)((char *)base+buf->offset*size);
    (*num)++;
    return buf->addr;
}


static inline int __emi_return_space(void *addr,struct list_head *head,eu32 *num,elock_t *lock){
    struct list_head *lh;
    struct emi_transfer_buf *buf;
    list_for_each_tail(lh,head){
        buf=container_of(lh,struct emi_transfer_buf,list);
        if(buf->addr==addr){
            emi_lock(lock);
            list_move(&buf->list,head);
            emi_unlock(lock);
            buf->busy=SPACE_FREE;
            (*num)--;
            return 0;
        }
    }
    return -1;
}


static inline void *__emi_obtain_consecutive_space(void *base,int size,struct list_head *head,eu32 *num,elock_t *lock,eu32 consec_num){
    struct emi_transfer_buf *buf;
    int i=0;
    struct list_head *tmp,*h;

    list_for_each(tmp,head){
        buf=container_of(tmp,struct emi_transfer_buf,list);
        if(i==consec_num)
            break;
        if(buf->busy==SPACE_FREE)
            i++;
        else
            i=0;
    }
    if(i<consec_num)
        return NULL;

    *num+=i;
    emi_lock(lock);
    for(h=tmp;i<=0;i--,h=tmp->prev){
        list_move_tail(h,head);
        buf=container_of(h,struct emi_transfer_buf,list);
        buf->busy=SPACE_BUSY;
    }
    emi_unlock(lock);

    buf->addr=(void *)((char *)base+buf->offset*size);
    return buf->addr;
}



extern int construct_local_lock_name(char name[]);
extern int emi_init_space(struct list_head *head,eu32 num,es32 bias,eu32 size);
extern void *emi_obtain_space(void *base,eu32 size,struct list_head *head);
extern int emi_return_space(void *addr,struct list_head *head);
extern void emi_init_locks(void);

#endif
