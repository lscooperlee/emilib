
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
#include <pthread.h>
#include <time.h>

#include "emi_msg.h"
#include "emi_hash.h"
#include "emi_shbuf.h"
#include "emi_sockr.h"
#include "emi_lock.h"
#include "emi_dbg.h"
#include "emi_config.h"
#include "emi_shmem.h"
#include "emi_thread.h"


static void *emi_receive_operation(void *client_sd);


static int core_shmid=-1;
static struct emi_shmem_mgr core_shmem_mgr;
static struct emi_thread_pool emi_core_pool;

void emi_release(void){
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

static int __emi_core(void);

int emi_core(struct emi_config *config){
    if(config)
        set_default_config(config);
    return __emi_core();
}

static int __emi_core(void){

    eu32 pid_max;
    struct sk_dpr *sd=NULL;

    if(init_msg_table_lock(msg_table_head, &msg_table_lock)){
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
        struct sk_dpr *client_sd;

        if((client_sd=emi_accept(sd,NULL))==NULL){
            emilog(EMI_WARNING, "emi_accept error\n");
            continue;
        }

        emi_thread_pool_submit(&emi_core_pool, (emi_thread_func)emi_receive_operation, (void *)client_sd);

    }
}

static void *emi_receive_operation(void *client_sd){
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
    if(emi_msg_read((struct sk_dpr *)client_sd, msg_pos) < 0){
        emilog(EMI_INFO, "emi_read from client error or emi_init is guessing port\n");
        goto e0;
    }


    if(msg_pos->flag&EMI_MSG_CMD_REGISTER){
        emilog(EMI_DEBUG, "Received a register msg with num %d and pid %d\n",
                msg_pos->msg, msg_pos->addr.pid);
        
        // FIXME
        struct msg_map *p = alloc_msg_map();
        if(p == NULL){
            goto e0;
        }

        msg_map_init(p,msg_pos->msg,msg_pos->addr.pid);

        /*
         * Make sure msg_map with the same pid and msgnum are not in msg_table,
         * this prevents one process register multiple functions to the same msg number.
         */
        if(emi_hinsert_unique_lock(msg_table_head, p) < 0){
            emilog(EMI_ERROR, "process with same msg number already in msg_map\n");
            msg_pos->flag &= ~EMI_MSG_RET_SUCCEEDED;
        }else{
            emi_mutex_init(&core_shmem_mgr.pididx_lock[p->pid]);
            msg_pos->flag |= EMI_MSG_RET_SUCCEEDED;
        }

        emilog(EMI_DEBUG, "Insert msg to msg_table done");
        if(emi_msg_write_payload((struct sk_dpr *)client_sd, msg_pos) < 0){
            emilog(EMI_ERROR, "emi read payload from client error\n");
        }

        emilog(EMI_DEBUG, "Register msg finished");

        goto e0;

    }else{

        int nth;
        eu32 *pid_num_addr;

        emilog(EMI_DEBUG, "Received a sending msg with num %d, data size = %d\n", msg_pos->msg, msg_pos->size);

        nth=get_shbuf_offset(core_shmem_mgr.base, msg_pos);

        struct hlist_node *tmp;
        struct msg_map *map;

        emi_hsearch(msg_table_head, map, node, msg_pos->msg){
            if(map->msg == msg_pos->msg){
                msg_pos->count++;
            }
        }

        int max_exit_receiver = msg_pos->count;
        if(msg_pos->count > 0) {
            msg_pos->flag |= EMI_MSG_RET_SUCCEEDED;

            emi_rwlock_rdlock(&msg_table_lock);

            emi_hsearch(msg_table_head, map, node, msg_pos->msg){
                if(map->msg == msg_pos->msg){
                    pid_num_addr = &core_shmem_mgr.pididx[map->pid];
                    emi_mutex_lock(&core_shmem_mgr.pididx_lock[map->pid]);
                    emilog(EMI_DEBUG, "pid %d found, for msg %d, cmd %d\n", map->pid, msg_pos->msg, msg_pos->cmd);

                    *pid_num_addr = nth;
                    int pid_ret=kill(map->pid,SIGUSR2);

                    if(pid_ret<0){
                        *pid_num_addr=0;
                    }

                    emilog(EMI_DEBUG, "kill return %d for pid %d\n", pid_ret, map->pid);
                   
                    while (*pid_num_addr) {
                        sched_yield(); //Need discussion
                    };

                    emi_mutex_unlock(&core_shmem_mgr.pididx_lock[map->pid]);
                }
            }

            emi_rwlock_unlock(&msg_table_lock);

            emilog(EMI_DEBUG, "Signal all processes with msg num %d, waiting for sync\n", msg_pos->msg);
            
            while(msg_pos->count){

                emi_rwlock_wrlock(&msg_table_lock);

                emi_hsearch_safe(msg_table_head, map, tmp, node, msg_pos->msg){
                    if(map->msg == msg_pos->msg){
                        int pid_ret=kill(map->pid, 0);
                        if(pid_ret<0){
                            emi_spin_lock(&msg_pos->lock);
                            msg_pos->count--;
                            emi_spin_unlock(&msg_pos->lock);

                            emilog(EMI_DEBUG, "receiver has gone, decrease msg count\n");

                            max_exit_receiver--;
                            free_msg_map(map);
                        }
                    }
                }

                emi_rwlock_unlock(&msg_table_lock);

                struct timespec req = {0, 1000*1000*10};
                nanosleep(&req, NULL);
            }

            if(max_exit_receiver == 0){ //all receivers are exit receiver, return false
                emilog(EMI_ERROR, "All receivers are exit receivers\n");
                msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
            }

        }else{
            emilog(EMI_ERROR, "msg count is zero, no receiver process responded\n");
            msg_pos->flag&=~EMI_MSG_RET_SUCCEEDED;
        }

        if(msg_pos->flag&EMI_MSG_MODE_BLOCK){
            emilog(EMI_DEBUG, "Return the state and possible data for block mode\n");

            if(emi_msg_write_ret((struct sk_dpr *)client_sd, msg_pos)< 0){
                emilog(EMI_ERROR, "Write data back to client\n");
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
    emi_close((struct sk_dpr *)client_sd);
    return NULL;
}
