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


void func_sterotype(int no_use){
    int id,nth,pid;
    struct emi_msg *shmsg,*baseshmsg;
    struct list_head *lh;

    id=emi_global.shm_id;

    if((baseshmsg=(struct emi_msg *)shmat(id,(const void *)0,0))==(void *)-1){
        dbg("shmat error, did you run emi_init() ?! \n");
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

    cmd.src_addr.ipv4.sin_addr.s_addr=inet_addr("127.0.0.1");
    cmd.src_addr.ipv4.sin_port=htons(emi_config->emi_port);
    cmd.src_addr.ipv4.sin_family=AF_INET;
    cmd.src_addr.pid=getpid();

    cmd.msg=defined_msg;
    cmd.flag=EMI_MSG_CMD_REGISTER;

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
