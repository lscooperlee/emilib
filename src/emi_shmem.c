/*
EMI:	embedded message interface
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
#include "shmem.h"
#include "emi_semaphore.h"

//#define GET_SIZE(type)	sizeof(type)


LIST_HEAD(__msg_list);
LIST_HEAD(__data_list);
eu32 __num_busy_data=0;
eu32 __num_busy_msg=0;

elock_t __emi_msg_space_lock;
elock_t __emi_data_space_lock;

elock_t msg_map_lock;
elock_t critical_shmem_lock;

//#ifdef DEBUG
void debug_emi_transfer_buf(struct emi_transfer_buf *buf){
	printf("debug_emi_transfer_buf:\n	");
	printf("buf->offset=%d,buf->busy=%d,buf->addr=%p\n",buf->offset,buf->busy,buf->addr);
}
//#endif

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


#if 0
void *emi_obtain_space(void *base,eu32 size,struct list_head *head){
	struct emi_transfer_buf *buf;
	buf=container_of(head->next,struct emi_transfer_buf,list);
	if(buf->busy)
		return NULL;
	emi_lock(&__emi_msg_space_lock);
	list_move_tail(&buf->list,head);
	buf->busy=SPACE_BUSY;
	emi_unlock(&__emi_msg_space_lock);
	buf->addr=(void *)((char *)base+buf->offset*size);
	return buf->addr;
}


int emi_return_space(void *addr,struct list_head *head){
	struct list_head *lh;
	struct emi_transfer_buf *buf;
	list_for_each_tail(lh,head){
		buf=container_of(lh,struct emi_transfer_buf,list);
		if(buf->addr==addr){
			emi_lock(&__emi_msg_space_lock);
			list_move(&buf->list,head);
			buf->busy=SPACE_FREE;
			emi_unlock(&__emi_msg_space_lock);
			return 0;
		}
	}
	return -1;
}
#endif

int construct_local_lock_name(char name[]){
	int j;
	char buf[]=LOCKBASENAME;
	int uid=getuid();
	memcpy(name,buf,sizeof(buf));
	for(j=sizeof(buf)-1;uid>0;j++){
		name[j]=uid%10+'0';
		uid=uid/10;
	}
	name[j]='\0';
	return 0;
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
/*__emi_data_space_lock is not used again,data space will alloc together with emi_msg space.*/
//	emi_lock_init(&__emi_data_space_lock);
	emi_lock_init(&msg_map_lock);
	emi_lock_init(&critical_shmem_lock);
};


#if 0
int emi_obtain_empty_space(void *base,int size){
	char *tmp;
	for(tmp=(char *)base;tmp!=NULL;){
	}
}
#endif
