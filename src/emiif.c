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
#include <sys/unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include "emi.h"
#include "emisocket.h"
#include "shmem.h"
#include "list.h"
#include "emi_semaphore.h"
#include "emi_config.h"
#include "emi_dbg.h"


#ifdef BLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#endif

struct emi_global{
    int shm_id;
    int sem_id;
    int urandom_fd;
}emi_global;

struct func_list{
    struct list_head list;
    emi_func func;
    eu32 msg;
    void *data;
};

LIST_HEAD(__func_list);


struct emi_addr *emi_addr_alloc(){
    struct emi_addr *addr;
    if((addr=(struct emi_addr *)malloc(sizeof(struct emi_addr)))==NULL){
        goto error;
    }
    memset(addr,0,sizeof(struct emi_addr));

    return addr;
error:
    return NULL;
}

void emi_addr_free(struct emi_addr *addr){

    free(addr);
    return;
}



struct emi_msg *emi_msg_alloc(eu32 size){

    struct emi_msg *msg;
    if((msg=(struct emi_msg *)malloc(sizeof(struct emi_msg)+size))==NULL){
        goto error;
    }
    memset(msg,0,sizeof(struct emi_msg));
    msg->size=size;

    return msg;

error:
    return NULL;
}

void emi_msg_free(struct emi_msg *msg){
    free(msg);
    return;
}


static int split_ipaddr(char *mixip,char *ip,int *port){
    int ret=0;
    char sip[32]={0},*p;
    strcpy(sip,mixip);
    p=strchr(sip,':');
    if(p){
        *p='\0';
        *port=atoi(++p);
    }else{
        *port=USR_EMI_PORT;
        ret=-1;
    }
    strcpy(ip,sip);
    return ret;
}

int emi_fill_addr(struct emi_addr *addr,char *ip,int port){

    if(((addr)->ipv4.sin_addr.s_addr=inet_addr(ip))==-1)
        return -1;

    (addr)->ipv4.sin_port=htons(port);
    (addr)->ipv4.sin_family=AF_INET;
    (addr)->pid=getpid();
    return 0;
}

int emi_fill_msg(struct emi_msg *msg,char *dest_ip,void *data,eu32 cmd,eu32 defined_msg,eu32 flag){
    struct emi_addr *p=&(msg)->src_addr;
    if(emi_fill_addr(p,"127.0.0.1",emi_config->emi_port))
        return -1;
    if(dest_ip!=NULL){
        char newip[16];
        int port;
        split_ipaddr(dest_ip,newip,&port);
        if(emi_fill_addr(&(msg->dest_addr),dest_ip,port))
            return -2;
    }
    if(data!=NULL){
//the massage could be zero,do not deem it as the end of the massage.
//        eu32 n=strlen(data);
//        n=(n<msg->size)?n:msg->size;
        memcpy((msg)->data,data,msg->size);
    }
    (msg)->cmd=cmd;
    (msg)->msg=defined_msg;
    (msg)->flag|=flag;
    return 0;
}

void func_sterotype(int no_use){
    int id,nth,pid;
    struct emi_msg *shmsg,*baseshmsg;
    struct list_head *lh;

    id=emi_global.shm_id;

    if((baseshmsg=(struct emi_msg *)shmat(id,(const void *)0,0))==(void *)-1){
        dbg("shmat error\n");
        exit(-1);        //error!!!!!!!!!!
    }

    pid=getpid();
    nth=*((eu32 *)baseshmsg+pid);

    shmsg=(struct emi_msg *)((char *)baseshmsg+nth);

    //this address is the pid_num in emi_core.it should be changed as soon as possable.emi_core will wait the zero to make sure the this process recieved the signal ,and send the same signal to other process that registered the same massage.
    *(eu32 *)((eu32 *)baseshmsg+pid)=0;


    list_for_each(lh,&__func_list){
        struct func_list *fl;
        fl=container_of(lh,struct func_list,list);
        if(fl->msg==shmsg->msg){

            if(shmsg->flag&EMI_MSG_MODE_BLOCK){
                int ret;
                ret=(fl->func)(shmsg);
//in BLOCK mode, if function return with error,then msg->flag must be set to FAILED to ensure the emi_core return failed result to the sender.
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


    //FIXME:lock
    shmsg->count--;
    //FIXME:unlock


    if(shmdt(baseshmsg)){
        dbg("shmdt error\n");
        return;
    }

    return;
}


int emi_msg_send(struct emi_msg *msg){
    struct sk_dpr *sd;
    int ret=-1;

    if((msg->flag&EMI_MSG_TYPE_DATA)&&(msg->data==NULL)){
        dbg("flag tells the msg includes data,but msg->data pointer is NULL\n");
        return -1;
    }
    if((sd=emi_open(AF_INET))==NULL){
        dbg("emi_open error\n");
        return -1;
    }
    if((emi_connect(sd,&(msg->dest_addr),1))<0){
        dbg("remote connected error\n");
        goto out;
    }

    if((emi_write(sd,(void *)msg,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
        dbg("block mode:error when emi_write to remote daemon\n");
        goto out;
    }

    if(msg->flag&EMI_MSG_TYPE_DATA){
        if((emi_write(sd,msg->data,msg->size))<msg->size){
            dbg("nonblock mode:DATA:emi_write to remote prcess local data with error\n");
            goto out;
        }
    }

        //waiting for optimizing
    if(msg->flag&EMI_MSG_MODE_BLOCK){

/*
 * keep consistent with the emi_core, try to read an emi_msg struct first, if the msg handler function returns an error, emi_core 
 *      will not send us the emi_msg struct, but we don't know that, so try to read it, if we read returns an error, that means
 *      emi_core has close the client_sd (accepted socket), some errors must be occured.
 *
 *         if we can read the emi_msg struct entirely, a SCCEEDED flag must be set, than we check if the
 *         returned msg->flag&EMI_MSG_TYPE_DATA, if true, emi_core must have been returned us extra data,
 *         we should receive it immediately.
 *
 *         there are two things that should be noticed.
 *
 *         first, currently
 *         the returned extra data will be not larger than the sent msg->size data, because we have no way to return this
 *         extra data ,so we will use msg->data to store the returned extra data, which means the returned data size should not
 *         large than the sent data size.
 *
 *         NOTE: THAT this will probably be changed in the future, if we use buddy allocator to manager the msg->data space,
 *         than we can return even bigger size of extra data back.
 *
 *
 *         second, currently
 *         this returned extra data is not required to be check and it will be probably received with no error,
 *         if we receive returned emi_msg, it must be a SCCEEDED state, we don't have to check SCCEEDED again for
 *         receiving extra data if it has.
 *         because all the  checks were done by the emi_msg_prepare_return and func_sterotype, see the two functions for details.
 *
 */
        if((emi_read(sd,msg,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
            dbg("block mode:read msg emi_read from remote process error\n");
            goto out;
        }
        if(msg->flag&EMI_MSG_TYPE_DATA){
            if((emi_read(sd,msg->data,msg->size))<msg->size){
                dbg("block mode:read extra data emi_read from remote process error\n");
                goto out;
            }
        }
        ret=0;
        dbg("a block msg sent successfully\n");
    }else{
        ret=0;
        dbg("a nonblock msg sent successfully\n");
    }
out:
    emi_close(sd);
    return ret;
}

static inline int emi_msg_send_highlevel(char *ipaddr, int msgnum,void *send_data,int send_size, void *ret_data,int ret_size, eu32 cmd,eu32 flag){
    int size=send_size>ret_size?send_size:ret_size;
    struct emi_msg *msg=emi_msg_alloc(size);
    if(msg==NULL){
        dbg("emi_msg_alloc error\n");
        return -1;
    }

    msg->size=send_size;
    emi_fill_msg(msg,ipaddr,send_data,cmd,msgnum,flag);
    if(emi_msg_send(msg)){
        dbg("emi_msg_send error\n");
        emi_msg_free(msg);
        return -1;
    }

    if(ret_data&&msg->size){
        memcpy(ret_data,msg->data,msg->size);
    }

    emi_msg_free(msg);
    return 0;
}

/*
 * NOTE:
 *     BE CAREFUL, the ret_size in this function is supposed to be the expected return data size from the receiver.
 *         normally this should be awared by both receiver and sender.
 *         the receiver can return data as much as possible, only limited by the emi max space range.
 *         but the sender can not know how much data the receiver will return, if the receiver returns a large amount of data,
 *         but the sender only prepares a small size buffer to store it, the sender will probably get some kind of system memory error.
 *
 *         this of course can be fixed by cycling read and realloc, but I don't want to do that recently.
 *
 */
int emi_msg_send_highlevel_block(char *ipaddr, int msgnum,void *send_data,int send_size,void *ret_data,  int ret_size,eu32 cmd){
    eu32 flag=EMI_MSG_MODE_BLOCK;
    if(send_data)
        flag|=EMI_MSG_TYPE_DATA;

    return emi_msg_send_highlevel(ipaddr,msgnum,send_data,send_size,ret_data,ret_size,cmd,flag);
}

int emi_msg_send_highlevel_nonblock(char *ipaddr, int msgnum,void *send_data, int send_size, eu32 cmd){
    eu32 flag=0;
    if(send_data)
        flag=EMI_MSG_TYPE_DATA;

    return emi_msg_send_highlevel(ipaddr,msgnum,send_data,send_size,NULL,0,cmd,flag);
}

/*
 * this function should be called inside msg handler function, preparing returned extra data, several steps:
 *      step 1: check if size > MAX_SIZE_THAT_CAN_BE_ALLOC, at this version emi_config->emi_data_size_per_msg,
 *      the ret data size can be larger than the received data size, an example is the sender has no data to be transmit to
 *      the receiver, thus the received data size should be zero, but need the receiver to write back an extra data, so the
 *      returned data size is larger than the received one.
 *
 *      step 2: change msg->flag to EMI_MSG_RET_SUCCEEDED|EMI_MSG_TYPE_DATA|EMI_MSG_MODE_BLOCK, see also func_sterotype
 *          SUCCEEDED tells the emi_core all is well, TYPE_DATA tells emi_core we have extra data to return,
 *          MODE_BLOCK tells emi_core that this is an block msg, this must be a block msg, or it won't return data back.
 *          normally msg->flag should already set that bit, here we just make it clear.
 *
 *      when using this function in msg handler, we must check the return value of this function and must not return 0(success)
 *      in msg handler when emi_msg_prepare_return_data return -1(fail)
 */
int emi_msg_prepare_return_data(struct emi_msg *msg,void *data,eu32 size){

    if(size>emi_config->emi_data_size_per_msg){
        dbg("the size of returned extra data is too large\n");
        msg->flag&=~EMI_MSG_RET_SUCCEEDED;
        return -1;
    }
    msg->size=size;
    memcpy(msg->data,data,size);
    /*here used to be a bug but now fixed, in nonblock mode, user may still misuse this function to return extra data to sender,
     * which is impossible because sender has already close the socket. flag should not be assigned directly like
     * msg->flag=EMI_MSG_RET_SUCCEEDED|EMI_MSG_TYPE_DATA|EMI_MSG_MODE_BLOCK,
     * the emi_core with check this flag, and do the prepared block issue even though the sender don't want to receive them
     */
    msg->flag|=EMI_MSG_RET_SUCCEEDED|EMI_MSG_TYPE_DATA;
    return 0;
}


static int __emi_msg_register(eu32 defined_msg,emi_func func){
    struct sk_dpr *sd;
    int ret;
    struct sigaction act, old_act;
    struct func_list *fl;
    struct emi_msg cmd;

    if((sd=emi_open(AF_INET))==NULL){
        dbg("emi_open error\n");
        return -1;
    }
    memset(&cmd,0,sizeof(struct emi_msg));
    if((ret=emi_fill_msg(&cmd,NULL,NULL,0,defined_msg,EMI_MSG_CMD_REGISTER))<0){
        emi_close(sd);
        dbg("emi_fill_msg error\n");
        return -1;
    }

    if((ret=emi_connect(sd,&cmd.src_addr,1))<0){
        emi_close(sd);
        dbg("emi_connect error\n");
        return -1;
    }
    if((ret=emi_write(sd,(void *)&cmd,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
        emi_close(sd);
        dbg("emi_write error\n");
        return -1;
    }
    if((ret=emi_read(sd,(void *)&cmd,sizeof(struct emi_msg)))<sizeof(struct emi_msg)){
        emi_close(sd);
        dbg("emi_read register back error\n");
        return -1;
    }
    if(!(cmd.flag&EMI_MSG_RET_SUCCEEDED)){
        dbg("emi returns an ~SUCCEEDED flag\n");
        return -1;
    }

    if((fl=(struct func_list *)malloc(sizeof(struct func_list)))==NULL){
        dbg("malloc error\n");
        return -ENOMEM;
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
    if((ret=sigaction(SIGUSR2,&act,&old_act))<0){
        dbg("sigaction error\n");
        return ret;
    }
    return 0;
}

int emi_msg_register(eu32 defined_msg,emi_func func){
    return __emi_msg_register(defined_msg,func);
}

void emi_msg_unregister(eu32 defined_msg,emi_func func){
    struct list_head *lh;
    list_for_each(lh,&__func_list){     //is this correct? NOTE THAT
        struct func_list *fl;
        fl=container_of(lh,struct func_list,list);
        if(NULL==func){
            if(defined_msg==fl->msg){
                list_del(lh);
                free(fl);
            }
        }else{
            if((defined_msg==fl->msg)&&(func==fl->func)){
                list_del(lh);
                free(fl);
                return;
            }
        }
    }
    return;

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

    if((emi_global.shm_id=shmget(emi_config->emi_key,0,0))<0){
        dbg("shmget error\n");
        return -1;
    }
    if((emi_global.urandom_fd=open("/dev/urandom",O_RDONLY))<0){
        dbg("urandom fd open error\n");
        return -1;
    }

    return 0;
}

void emi_loop(void){
    while(1){
        pause();
    }
}

/**
 *Note: these functions are used for transporting emilib into other programming language like Python, it is more convienent to just 
     use basic types when transporting (int, char and all kinds of pointers ) rather than structure. many transporting methods may
    be hard to deal with complicated C types. these functions are much useful and can make things easer.

    of course you can use them in C too.
 *
 */
eu32 get_msgnum_from_msg(struct emi_msg *msg){
    return msg->msg;
}
eu32 get_cmd_from_msg(struct emi_msg *msg){
    return msg->cmd;
}

char *get_data_from_msg(struct emi_msg *msg){
    return msg->data;
}
eu32 get_datasize_from_msg(struct emi_msg *msg){
    return msg->size;
}

void copy_data_from_msg(struct emi_msg *msg,void *dest){
    memcpy(dest,msg->data,msg->size);
}
