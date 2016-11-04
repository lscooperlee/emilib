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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>

#include "emi.h"
#include "emi_sock.h"
#include "emi_shbuf.h"
#include "list.h"
#include "emi_semaphore.h"
#include "emi_config.h"
#include "emi_dbg.h"
#include "emi_shmem.h"
#include "emi_core.h"

struct emi_global{
    int shm_id;
}emi_global;

struct func_list{
    struct list_head list;
    emi_func func;
    eu32 msg;
    void *data;
};

LIST_HEAD(__func_list);


void func_sterotype(int no_use){
    int id,nth,pid;
    struct emi_msg *shmsg;
    eu32 *baseshmsg;
    struct list_head *lh;
    void *base;

    id=emi_global.shm_id;

    if((base=(void *)emi_shm_alloc(id, EMI_SHM_READ|EMI_SHM_WRITE))==NULL){
        dbg("emi_shm_alloc error, did you run emi_init() ?! \n");
        exit(-1);
    }

    pid=getpid();
    baseshmsg = GET_PIDIDX_BASE(base);
    nth=baseshmsg[pid];

    shmsg = (struct emi_msg *)get_shbuf_addr(base, nth);

    //this address is the pid_num in emi_core.it should be changed as soon as possable.emi_core will wait the zero to make sure the this process recieved the signal ,and send the same signal to other process that registered the same massage.
    baseshmsg[pid] = 0;

    //For the possible user retdata allocation
    void *emibuf_top = GET_EMIBUF_BASE(base);
    espinlock_t *lock = GET_EMIBUF_LOCK_BASE(base);
    update_emi_buf_lock(base, emibuf_top, lock);

    put_msg_data_addr(shmsg);

    list_for_each(lh,&__func_list){
        struct func_list *fl;
        fl=container_of(lh,struct func_list,list);
        if(fl->msg==shmsg->msg){

            if(shmsg->flag&EMI_MSG_MODE_BLOCK){
                int ret;
                ret=(fl->func)(shmsg);

                //in BLOCK mode, if function return with error,
                //then msg->flag must be set to FAILED to ensure the emi_core return failed result to the sender.
                if(ret){
                    dbg("the msg handler returned an error ret=%d\n",ret);
                    shmsg->flag&=~EMI_MSG_RET_SUCCEEDED;
                }else{
                    shmsg->flag|=EMI_MSG_RET_SUCCEEDED;
                }
                
                //should break here because BLOCK msg is an exclusive msg.
                break;
            }else{
                (fl->func)(shmsg);
            }

        }
    }

    //must be called before semaphore, emi_core should continue after msg->data has been reset to offset
    put_msg_data_offset(shmsg);

    //semaphore with emi_core
    //FIXME: lock and unlock. Multiple process may register the same message, so may modify the shared emi_msg
    //at the same time.
    emi_spin_lock(&shmsg->lock);
    shmsg->count--;
    emi_spin_unlock(&shmsg->lock);

    if(emi_shm_free(base)){
        dbg("emi_shm_free error\n");
        return;
    }

    return;
}

static int __emi_msg_register(eu32 defined_msg,emi_func func){
    struct sk_dpr *sd;
    int ret = -1;
    struct sigaction act, old_act;
    struct func_list *fl;
    struct emi_msg cmd;

    if((sd=emi_open(AF_INET))==NULL){
        dbg("emi_open error\n");
        return -1;
    }
    memset(&cmd,0,sizeof(struct emi_msg));

    cmd.addr.ipv4.sin_addr.s_addr=inet_addr("127.0.0.1");
    cmd.addr.ipv4.sin_port=htons(emi_config->emi_port);
    cmd.addr.ipv4.sin_family=AF_INET;
    cmd.addr.pid=getpid();

    cmd.msg=defined_msg;
    cmd.flag=EMI_MSG_CMD_REGISTER;

    if(emi_connect(sd,&cmd.addr,1)){
        goto out;
    }

    if(emi_msg_write_payload(sd,&cmd)){
        goto out;
    }

    if(emi_msg_read_payload(sd, &cmd)){
        goto out;
    }

    if(!(cmd.flag&EMI_MSG_RET_SUCCEEDED)){
        goto out;
    }

    if((fl=(struct func_list *)malloc(sizeof(struct func_list)))==NULL){
        goto out;
    }
    memset(fl,0,sizeof(struct func_list));
    fl->func=func;
    fl->msg=defined_msg;
    list_add(&fl->list,&__func_list);

    act.sa_handler=func_sterotype;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask,SIGUSR2);
//    act.sa_flags=SA_INTERRUPT;
    act.sa_flags= SA_RESTART;    //this is important, RESTART system call during signal handler.
    if(sigaction(SIGUSR2,&act,&old_act)){
        goto out;
    }
    
    ret = 0;
out:
    emi_close(sd);
    return ret;
}

int emi_msg_register(eu32 defined_msg,emi_func func){
    return __emi_msg_register(defined_msg,func);
}

void *emi_retdata_alloc(struct emi_msg *msg, eu32 size){
    void *addr = msg->data;

    if (!(msg->flag & EMI_MSG_MODE_BLOCK)) {
        dbg("an ~BLOCK msg is sent, receiver is not expecting receive data");
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        return NULL;
    }

    if(size > msg->size){
        addr = emi_alloc(size);
        if(addr == NULL){
            msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
            return NULL;
        }

        free_shared_msg_data(msg);
        msg->data = addr;
    }

    msg->size = size;
    
    return addr;
}

int emi_msg_prepare_return_data(struct emi_msg *msg, void *data, eu32 size) {
    void *retdata = emi_retdata_alloc(msg, size);
    if(retdata == NULL){
        return -1;
    }

    memcpy(retdata, data, size);
    return 0;
}

/*
 *emi_init must be used before recieving process.it uses the emi_config struct, which may requre emi config file.
 *
 * */
int emi_init(){
    struct emi_config *config=get_config();
    if(config){
        set_default_config(config);
    }else{
        config=guess_config();
        if(config==NULL)
            return -1;
        set_default_config(config);
    }

    eu32 pid_max = get_pid_max();

    if((emi_global.shm_id=emi_shm_init("emilib", GET_SHM_SIZE(pid_max), 0))<0){
        dbg("emi_shm_init error\n");
        return -1;
    }

    return 0;
}

void emi_loop(void){
    while(1){
        pause();
    }
}

