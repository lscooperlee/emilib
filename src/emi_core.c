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

#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>

#include "emi.h"
#include "msg_table.h"
#include "emi_shbuf.h"
#include "emisocket.h"
#include "emi_semaphore.h"
#include "emi_dbg.h"
#include "emi_config.h"
#include "emi_shmem.h"

#define coreprt(a) emiprt(a)


struct clone_args{
    struct sk_dpr *sd;
    struct sk_dpr *client_sd;
};

static int emi_recieve_operation(void *args);


static int core_shmid=-1;
static struct sk_dpr *sd=NULL;
static struct sk_dpr *client_sd=NULL;
static eu32 *emi_base_addr = NULL;
static void *emi_shbuf_base_addr = NULL;
static struct msg_map *msg_table[EMI_MSG_TABLE_SIZE];

elock_t msg_map_lock;
elock_t critical_shmem_lock;

void emi_init_locks(void){
    emi_lock_init(&msg_map_lock);
    emi_lock_init(&critical_shmem_lock);
};


void emi_release(void){
    emi_close(sd);
    emi_close(client_sd);
    if(core_shmid>=0){
        emi_shm_destroy("emilib", core_shmid);
    }
}


static int init_msg_table(struct msg_map *table[]){
    int i;
    for(i=0;i<EMI_MSG_TABLE_SIZE;i++)
        table[i]=NULL;
    return 0;
}



static int int_global_shm_space(int pid_max){
    if((core_shmid=emi_shm_init("emilib", pid_max * sizeof(eu32) + (BUDDY_SIZE << EMI_ORDER_NUM), EMI_SHM_CREATE))<0){
        coreprt("emi_shm_init error\n");
        return -1;
    }
    if((emi_base_addr=(void *)emi_shm_alloc(core_shmid, EMI_SHM_READ|EMI_SHM_WRITE))==NULL){
        coreprt("emi_shm_alloc error\n");
        emi_shm_destroy("emilib", core_shmid);
        return -1;
    }

    emi_shbuf_base_addr = (void *)emi_base_addr + pid_max * sizeof(eu32);

    return 0;
}

eu32 get_pid_max(void){
    int fd,i;
    char buf[8]={0};
    if((fd=open("/proc/sys/kernel/pid_max",O_RDONLY))<0){
        goto error;
    }
    if(read(fd,buf,sizeof(buf))<0){
        close(fd);
        goto error;
    }
    close(fd);
    i=atoi(buf);
    return i;

error:
    perror("dangerous!it seems your system does not mount the proc filesystem yet,so can not get your pid_max number.emi_core would use default ,but this may be different with the value in your system,as a result,may cause incorrect transmission");
    return 32768;
}

static int __emi_core(void);

int emi_core(struct emi_config *config){
    if(config)
        set_default_config(config);
    return __emi_core();
}

static int __emi_core(void){

    eu32 pid_max;
    int ret;

    if(init_msg_table(msg_table)){
        coreprt("init msg table error\n");
        return -1;
    }

    pid_max=get_pid_max();

    emi_init_locks();

    if(int_global_shm_space(pid_max)){
        coreprt("init shm space error\n");
        return -1;
    }

    if(init_emi_buf(emi_shbuf_base_addr)){
        coreprt("init msg space error\n");
        return -1;
    }

    if((sd=emi_open(AF_INET))==NULL){
        coreprt("emi_open error\n");
        emi_shm_destroy("emilib", core_shmid);
        return -1;
    }

    int yes=1;
    if(setsockopt(sd->d, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        coreprt("setsockopt error");
        return -1;
    }


    if(emi_bind(sd,emi_config->emi_port)<0){
        coreprt("bind error\n");
        emi_close(sd);
        emi_shm_destroy("emilib", core_shmid);
        return -1;
    }

    if(emi_listen(sd)<0){
        coreprt("listen err\n");
        emi_close(sd);
        emi_shm_destroy("emilib", core_shmid);
        return -1;
    }


    while(1){
        if((client_sd=emi_accept(sd,NULL))==NULL){
            coreprt("emi_accept error\n");
            emi_close(client_sd);
            continue;
        }

        pthread_t tid;
        struct clone_args *arg;

        if((arg=(struct clone_args *)malloc(sizeof(struct clone_args)))==NULL){
            coreprt("mem error\n");
            continue;
        }

        arg->client_sd=client_sd;

        if((ret=pthread_create(&tid,NULL,(void *)emi_recieve_operation,arg))){
            coreprt("pthread cancel error\n");
            continue;
        }

        if((ret=pthread_detach(tid))){
            coreprt("pthread_detach error\n");
            continue;
        }
    }
}

static int emi_recieve_operation(void *args){
    int ret,pid_ret=-1;
    struct emi_msg *msg_pos;

/*
 * get an empty area in the share memory for a recieving msg.
*/
    if((msg_pos=emi_msg_shbuf_alloc())==NULL){
        coreprt("emi_obtain_msg_space error\n");
        goto e0;
    }

/*
 * read the remote msg into this alloced memory, if this one got an error, probably emi_init has sent a guess message to emi_core, which
 * is a connection to emi_core without any data transfered.
 */
    if((ret=emi_read(((struct clone_args *)args)->client_sd,msg_pos,sizeof(struct emi_msg)))<(sizeof(struct emi_msg))){
        coreprt("emi_read from client error or emi_init is guessing port\n");
        goto e0;
    }

    debug_msg(msg_pos,0);

/*
 *    if it is a register msg ,then:
 */
    if(msg_pos->flag&EMI_MSG_CMD_REGISTER){
        coreprt("receive a register msg\n");
        struct msg_map p;
        msg_map_fill(&p,msg_pos->msg,msg_pos->src_addr.pid);
        emi_lock(&msg_map_lock);
        emi_hinsert(msg_table,&p);
        emi_unlock(&msg_map_lock);

        /*
         * for register a non block mode, the default flag should always be SUCCEEDED, for the block mode, we should also assume that
         *  the register process will succeed, unless msg_pos (mp in the next few lines) point to somebody, which means the same msg
         *  has allocated to another one.
         */
        msg_pos->flag|=EMI_MSG_RET_SUCCEEDED;
/*
 *    for EMI_MSG_MODE_BLOCK mode, we should search the whole msg_table to ensure this msg does not been registered before,
 */
        if(msg_pos->flag&EMI_MSG_MODE_BLOCK){
            coreprt("the received msg is an block msg\n");
            struct msg_map *mp;
            int tmpnum=0;
            mp=__emi_hsearch(msg_table,&p,&tmpnum);
            if(mp!=NULL){
                msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
            }
        }


//tell the process the registeration result
        if((ret=emi_write(((struct clone_args *)args)->client_sd,msg_pos,sizeof(struct emi_msg)))<(sizeof(struct emi_msg))){
            coreprt("emi_read from client error\n");
        }

        goto e0;

    }else{
        coreprt("receive an send msg\n");
/*
 * otherwise, we should operate it carefully. if the msg is a data msg, we should recieve the date first.
 */
        int num;
        struct msg_map p,*m;
        int nth;
        eu32 *pid_num_addr;

        if(msg_pos->size > 0){
            coreprt("this is a send msg with data \n");
            
            /*
             * now emi_core could receive arbitary data as long as emi_core has enough memory to hold it.
             */
            msg_pos = emi_msg_shbuf_realloc(msg_pos);
            if (msg_pos != NULL) {

                if ((ret = emi_read(((struct clone_args *) args)->client_sd,
                        msg_pos->data, msg_pos->size)) < msg_pos->size) {
                    coreprt("emi_read from client error\n");
                    goto e0;
                }

            }else{
                /*
                 * we should do something, for example write back a emi_msg hint that the data size exceeds a default one.
                 * but this won't work, you can only write back when the sender is ready to read, this is not the case for the sender
                 * because the sender may write us a ~BLOCK msg which is an asynchronized one (read nothing from emi_core)
                 *
                 * here, simply close(client_fd) would help. for ~BLOCK mode, the sender does not care, for BLOCK mode,
                 * the sender always read a result (either with or without an extra data), which means the read function in sender's
                 * code would return an error code, indicating a lack of memory.
                 */
                    goto e0;
            }
        }

        debug_msg(msg_pos,1);

/*
 *    get the offset of the msg in "msg split" area (see develop.txt). this offset will be writed into the BASE_ADDR+pid address afterward, inform the according process to get it.
 */
        nth=emi_get_space_msg_num(emi_base_addr, msg_pos);

        msg_map_fill(&p,msg_pos->msg,0);

/*
 *        each cycle would find a msg_map associated with corresponding MSG NUMBER.
 */
        for(num=0;;num+=1){
            emi_lock(&msg_map_lock);
            if((m=__emi_hsearch(msg_table,&p,&num))==NULL){
                emi_unlock(&msg_map_lock);
                break;
            }
            emi_unlock(&msg_map_lock);

/*
 * pid_num_addr is the index for emi_msg space in share memory
 * It is used for passing the number which tells the recievers
 * where the transfered msg could be found,at the same time, 
 * as a flag judged by emi_core weather the reciever has finished operating it,
 * which means that it is safe for emi_core to unlock and to reuse it again.
 * so it is quite critial.
 *
 * moreover,pid_num must be identified no less than EMI_MAX_MSG messages,
 * thus may be not enough if defined as a char type sometimes.
 * so here we treat as an int.
 *
 */
            /*get process id address in critical shmem index area*/
            pid_num_addr = emi_base_addr + m->pid;

            emi_lock(&critical_shmem_lock);//FIXME:this is ugly,all process conpete the only lock.

            /*write emi_msg space offset to the index address.*/
            *pid_num_addr = nth;
            pid_ret=kill(m->pid,SIGUSR2);
            if(pid_ret<0){
                *pid_num_addr = 0;
            }else{
                msg_pos->count++;
            }

            /*
             * if pid_num == nth, means the the SIGUSR2 is sent successfully.
             * So here we are waiting for the signal handler (func_sterotype)
             * to release the pid_num index.
             */
            while(*pid_num_addr){
                sleep(0);
            };

            emi_unlock(&critical_shmem_lock);

            /*
             * if pid_ret < 0, means no process with pid_num.
             * This is not an error, eg, the receiver process might just exit
             * by some reason. It only means the emi_core kept an redundant map.
             * So we should delete the redundance and continue
             */
            if(pid_ret<0){
                emi_lock(&msg_map_lock);
                emi_hdelete(msg_table,m);
                emi_unlock(&msg_map_lock);
            }
        }

        /*
         * Waiting for receiver process to finish all job.
         * Target process need to down this count once finishing the operation.
         * remember to lock and unlock when down the count.
         */
        while(msg_pos->count){
            sleep(0);
        }

        /*
         * After the receiver process finishes everything.
         * We should check if that's an ~BLOCK msg or BLOCK msg.
         * If it's an ~BLOCK msg, then simple return
         * If it's and BLOCK msg, we should prepare the return data that
         * the sender is expecting.
         */
        if(msg_pos->flag&EMI_MSG_MODE_BLOCK){
            coreprt("A block message, return states to the sender \n");
/*
 * In BLOCK mode, if the result is ~SUCCEEDED (lack of memory, msg handler function failed etc.),
 *  goto e0 and close(client_fd) immediately, though the sender is expecting return info from emi_core, we send nothing.
 *  just close the socket. The sender will get an error code as the return value of read function, indicating some errors occured.
 *
 *  if we are lucky enough that everything goes perfectly well, an SUCCEEDED flag will be set.
 *  In this condition, first we write msg_pos (an emi_msg struct) back to the sender. That means
 *  the receiver could make use of this feature to return data to the sender by putting it to
 *  emi_msg strcture body.
 *
 *  if EMI_MSG_RET_WITHDATA is set and msg_pos->size > 0, we send extra data (msg_pos->data) back,
 */
            if (msg_pos->flag & EMI_MSG_RET_SUCCEEDED) {
                if ((ret = emi_write(((struct clone_args *) args)->client_sd,
                        msg_pos, sizeof(struct emi_msg)))
                        < sizeof(struct emi_msg)) {
                    goto e0;
                }

                if ((msg_pos->flag & EMI_MSG_RET_WITHDATA) && (msg_pos->size > 0)) {
                    coreprt("Emi message handler returns extra data \n");
                    if ((ret = emi_write(
                            ((struct clone_args *) args)->client_sd,
                            msg_pos->data, msg_pos->size)) < msg_pos->size) {
                        goto e0;
                    }
                }

            } else {
                coreprt("Emi message handler returns a ~SUCCEEDED state\n");
                goto e0;
            }

        }else{
            coreprt("this send msg is an nonblock msg\n");
/*
 * in ~BLOCK mode, the logic here is very simple, just do the release work and return.
 */
            goto e0;
        }
    }

e0:
    emi_close(((struct clone_args *)args)->client_sd);
    emi_msg_shbuf_free(msg_pos);
    free(args);
    pthread_exit(NULL);
    return ret;

}
