
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
#include "emi_sock.h"
#include "emi_semaphore.h"
#include "emi_dbg.h"
#include "emi_config.h"
#include "emi_shmem.h"
#include "emi_thread.h"


struct clone_args{
    struct sk_dpr *sd;
    struct sk_dpr *client_sd;
};

static int emi_receive_operation(void *args);


static int core_shmid=-1;
static struct sk_dpr *sd=NULL;
static struct sk_dpr *client_sd=NULL;

static struct emi_shmem_mgr core_shmem_mgr;

static struct msg_map *msg_table[EMI_MSG_TABLE_SIZE];

static struct emi_thread_pool emi_core_pool;

void emi_release(void){
    emi_close(sd);
    emi_close(client_sd);
    if(core_shmid>=0){
        emi_shm_destroy("emilib", core_shmid);
    }
}


static int int_global_shm_space(int pid_max){
    void *base;

    if((core_shmid=emi_shm_init("emilib", GET_SHM_SIZE(pid_max), EMI_SHM_CREATE))<0){
        emilog(EMI_ERROR, "emi_shm_init error\n");
        return -1;
    }

    if((base=(void *)emi_shm_alloc(core_shmid, EMI_SHM_READ|EMI_SHM_WRITE))==NULL){
        emilog(EMI_ERROR, "emi_shm_alloc error\n");
        emi_shm_destroy("emilib", core_shmid);
        return -1;
    }

    emi_shmem_mgr_init(&core_shmem_mgr, base, pid_max);

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

    if(init_msg_table_lock(msg_table)){
        emilog(EMI_ERROR, "init msg table error\n");
        return -1;
    }

    pid_max=get_pid_max();

    if(int_global_shm_space(pid_max)){
        emilog(EMI_ERROR, "init shm space error\n");
        return -1;
    }

    if(init_emi_buf_lock(core_shmem_mgr.base, core_shmem_mgr.msgbuf, core_shmem_mgr.msgbuf_lock)){
        emilog(EMI_ERROR, "init msg space error\n");
        return -1;
    }

    if(emi_thread_pool_init(&emi_core_pool, 5)){
        emilog(EMI_ERROR, "init emi thread pool error\n");
        return -1;
    }

    if((sd=emi_open(AF_INET))==NULL){
        emilog(EMI_ERROR, "emi_open error\n");
        return -1;
    }

    int yes=1;
    if(setsockopt(sd->d, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        emilog(EMI_ERROR, "setsockopt error");
        return -1;
    }


    if(emi_bind(sd,emi_config->emi_port)<0){
        emilog(EMI_ERROR, "bind error\n");
        emi_close(sd);
        return -1;
    }

    if(emi_listen(sd)<0){
        emilog(EMI_ERROR, "listen err\n");
        emi_close(sd);
        return -1;
    }

    while(1){
        if((client_sd=emi_accept(sd,NULL))==NULL){
            emilog(EMI_WARNING, "emi_accept error\n");
            emi_close(client_sd);
            continue;
        }

        struct clone_args *arg;

        if((arg=(struct clone_args *)malloc(sizeof(struct clone_args)))==NULL){
            emilog(EMI_WARNING, "mem error\n");
            continue;
        }

        arg->client_sd=client_sd;

        emi_thread_pool_submit(&emi_core_pool, (void *)emi_receive_operation, arg);


   /* 
        pthread_t tid;

        if(pthread_create(&tid,NULL,(void *)emi_receive_operation,arg)){
            emilog(EMI_WARNING, "pthread cancel error\n");
            continue;
        }

        if(pthread_detach(tid)){
            emilog(EMI_WARNING, "pthread_detach error\n");
            continue;
        }
        */
    
    }
}

static int emi_receive_operation(void *args){
    int ret = -1;
    struct emi_msg *msg_pos;

/*
 * get an empty area in the share memory for a recieving msg.
*/
    if((msg_pos=alloc_shared_msg(0))==NULL){
        emilog(EMI_WARNING, "emi_obtain_msg_space error\n");
        goto e1;
    }

/*
 * read the remote msg into this alloced memory, if this one got an error, probably emi_init has sent a guess message to emi_core, which
 * is a connection to emi_core without any data transfered.
 */
    if((ret=emi_msg_read(((struct clone_args *)args)->client_sd,msg_pos)) < 0){
        emilog(EMI_INFO, "emi_read from client error or emi_init is guessing port\n");
        goto e0;
    }


    if(msg_pos->flag&EMI_MSG_CMD_REGISTER){
        emilog(EMI_DEBUG, "Received a register msg with num %d and pid %d\n",
                msg_pos->msg, msg_pos->addr.pid);

        struct msg_map p;
        msg_map_init(&p,msg_pos->msg,msg_pos->addr.pid);

        /*
         * Make sure msg_map with the same pid and msgnum are not in msg_table,
         * this prevents one process register multiple functions to the same msg number.
         */
        if((ret = emi_hinsert_lock(msg_table,&p)) < 0){
            msg_pos->flag &= ~EMI_MSG_RET_SUCCEEDED;
        }else{
            emi_lock_init(&core_shmem_mgr.pididx_lock[p.pid]);
            msg_pos->flag |= EMI_MSG_RET_SUCCEEDED;
        }

        emilog(EMI_DEBUG, "Insert msg to msg_table done");
        if((ret=emi_msg_write_payload(((struct clone_args *)args)->client_sd,msg_pos)) < 0){
            emilog(EMI_WARNING, "emi read payload from client error\n");
        }

        emilog(EMI_DEBUG, "Register msg finished");

        goto e0;

    }else{

        struct msg_map p;
        int nth;
        eu32 *pid_num_addr;
        int num;
        struct msg_map *map;

        emilog(EMI_DEBUG, "Received a sending msg with num %d, data size = %d\n", msg_pos->msg, msg_pos->size);
        debug_emi_msg(msg_pos);

        nth=get_shbuf_offset(core_shmem_mgr.base, msg_pos);

        msg_map_init(&p,msg_pos->msg,0);

        //msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
        msg_pos->flag |= EMI_MSG_RET_SUCCEEDED;

        for(num=0;;num+=1){
            emi_spin_lock(&msg_table_lock);
            if((map=__emi_hsearch(msg_table,&p,&num))==NULL){
                emi_spin_unlock(&msg_table_lock);

                //if no registered msg in msg_table, return failed
                if(num == 0)
                    msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
                break;
            }
            emi_spin_unlock(&msg_table_lock);

            pid_num_addr = &core_shmem_mgr.pididx[map->pid];
            emi_lock(&core_shmem_mgr.pididx_lock[map->pid]);
            emilog(EMI_DEBUG, "pid %d found, for msg %d, cmd %d\n", map->pid, msg_pos->msg, msg_pos->cmd);

            *pid_num_addr = nth;
            int pid_ret=kill(map->pid,SIGUSR2);
            if(pid_ret<0){
                *pid_num_addr=0;
                //emi_spin_lock(&msg_pos->lock);
                //msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
                //emi_spin_unlock(&msg_pos->lock);
                emi_hdelete_lock(msg_table,map);

            }else{
                emi_spin_lock(&msg_pos->lock);
                msg_pos->count++;
                emi_spin_unlock(&msg_pos->lock);
            }
            emilog(EMI_DEBUG, "kill return %d for pid %d\n", pid_ret, map->pid);

            while (*pid_num_addr) {
                sched_yield(); //Need discussion
            };

            emi_unlock(&core_shmem_mgr.pididx_lock[map->pid]);
        }

        emilog(EMI_DEBUG, "Signal all processes with msg num %d, waiting for sync\n", msg_pos->msg);
        while(msg_pos->count){
            sched_yield();
        }

        emilog(EMI_DEBUG, "msg_pos->flag = %x\n", msg_pos->flag);
        if(msg_pos->flag&EMI_MSG_MODE_BLOCK){
            emilog(EMI_DEBUG, "Return the state and possible data for block mode\n");

            if((ret = emi_msg_write_ret(((struct clone_args *) args)->client_sd, msg_pos))< 0){
                goto e0;
            }

        }else{
            emilog(EMI_DEBUG, "Return immediately for ~block mode\n");
            goto e0;
        }
    }

e0:
    free_shared_msg(msg_pos);
e1:
    emi_close(((struct clone_args *)args)->client_sd);
    free(args);
    //pthread_exit(NULL);
    return ret;

}
