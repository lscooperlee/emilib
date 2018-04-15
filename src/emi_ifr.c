
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
#include <time.h>

#include "emi_ifr.h"
#include "emi_msg.h"
#include "emi_sock.h"
#include "emi_shbuf.h"
#include "list.h"
#include "emi_lock.h"
#include "emi_config.h"
#include "emi_dbg.h"
#include "emi_shmem.h"
#include "emi_core.h"
#include "emi_thread.h"

struct emi_global{
    int shm_id;
}emi_global;

struct func_list{
    struct list_head list;
    emi_func func;
    eu32 msg;
};

espinlock_t __func_list_lock;
LIST_HEAD(__func_list);

static struct emi_thread_pool __recv_pool;
static struct emi_shmem_mgr __recv_mgr;
static eu32 __recv_pid;

void *__func_sterotype(void *args);
void func_sterotype(){
    int nth;
    nth=__recv_mgr.pididx[__recv_pid];
    struct emi_msg *shmsg;

    shmsg = (struct emi_msg *)get_shbuf_addr(__recv_mgr.base, nth);

    //this address is the pid_num in emi_core.it should be changed as soon as possable.emi_core will wait the zero to make sure the this process recieved the signal ,and send the same signal to other process that registered the same massage.
    __recv_mgr.pididx[__recv_pid] = 0;

    emi_thread_pool_submit(&__recv_pool, __func_sterotype, (void *)shmsg);

}

void *__func_sterotype(void *args){
    struct emi_msg *shmsg = (struct emi_msg *)args;
    struct list_head *lh;

    //For the possible user retdata allocation
    update_emi_buf_lock(__recv_mgr.base, __recv_mgr.msgbuf, __recv_mgr.msgbuf_lock);

    int ret=-1;
    emi_func func_ptr=NULL;
    
    emi_spin_lock(&__func_list_lock); 
    list_for_each(lh,&__func_list){
        struct func_list *fl;
        fl=container_of(lh,struct func_list,list);
        if(fl->msg==shmsg->msg){
            emilog(EMI_DEBUG, "Registerd function with msg num %d founded\n", shmsg->msg);
            func_ptr = fl->func;

            //One function for one message in one process
            break;
        }
    }
    emi_spin_unlock(&__func_list_lock);

    if(func_ptr != NULL){
        ret=func_ptr(shmsg);
        emilog(EMI_DEBUG, "Handler for msg %d done, %d returned\n", shmsg->msg, ret);
    }else{
        emilog(EMI_DEBUG, "Registerd function with msg num %d not found\n", shmsg->msg);
    }

    emi_spin_lock(&shmsg->lock);

    //Don't need lock here, no one changes this bit.
    if(shmsg->flag&EMI_MSG_MODE_BLOCK){

        if(ret){
            emilog(EMI_DEBUG, "msg handler running failed (not return zero)");
            shmsg->flag&=~EMI_MSG_RET_SUCCEEDED;
        //}else{
            // shmsg->flag|=EMI_MSG_RET_SUCCEEDED;
        }
    }
    
    emilog(EMI_DEBUG, "shmsg->count = %d\n", shmsg->count);

    shmsg->count--;
    emi_spin_unlock(&shmsg->lock);

    return NULL;
}

static int __emi_msg_register(eu32 defined_msg,emi_func func, eu32 flag){
    struct sk_dpr *sd;
    int ret = -1;
    struct func_list *fl;
    struct emi_msg cmd;


    if((sd=emi_open(AF_INET))==NULL){
        return -1;
    }
    memset(&cmd,0,sizeof(struct emi_msg));

    cmd.addr.ipv4.sin_addr.s_addr=inet_addr("127.0.0.1");
    cmd.addr.ipv4.sin_port=htons(emi_config->emi_port);
    cmd.addr.ipv4.sin_family=AF_INET;
    cmd.addr.pid=getpid();

    cmd.msg=defined_msg;
    cmd.flag=flag | EMI_MSG_CMD_REGISTER;

    if(emi_connect(sd,&cmd.addr)){
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

    // register function operates on list_head,
    // the register process may also register other msg, 
    // whose msg handler with be called on other thread, 
    // this msg handler will also operate on list_head,
    // therefore lock is needed.
    emi_spin_lock(&__func_list_lock); 
    list_add(&fl->list,&__func_list);
    emi_spin_unlock(&__func_list_lock);

    ret = 0;

    emilog(EMI_INFO, "Emi register done");

out:
    emi_close(sd);
    return ret;
}

int emi_msg_register(eu32 defined_msg,emi_func func){
    return __emi_msg_register(defined_msg,func, 0);
}

void *emi_retdata_alloc(struct emi_msg const *cmsg, eu32 size){

    struct emi_msg *msg = (struct emi_msg *)cmsg;

    if(size == 0){
        return NULL;
    }

    emi_spin_lock(&msg->lock);

    if (!(msg->flag & EMI_MSG_MODE_BLOCK)) {
        emilog(EMI_DEBUG, "the sended msg is not block message");
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        emi_spin_unlock(&msg->lock);
        return NULL;
    }

    struct emi_retdata *addr = (struct emi_retdata *)emi_alloc(size + sizeof(struct emi_retdata));
    if(addr == NULL){
        emilog(EMI_DEBUG, "emi_alloc error");
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        emi_spin_unlock(&msg->lock);
        return NULL;
    }

    eu32 old_retdata_offset = msg->retdata_offset;

    msg->retsize += size + sizeof(struct emi_retdata);
    msg->retdata_offset = GET_OFFSET(msg, addr);

    addr->next_offset = old_retdata_offset;
    addr->size = size;

    emi_spin_unlock(&msg->lock);
    
    return addr->data;
}

int emi_load_retdata(struct emi_msg const *msg, void const *data, eu32 size) {
    
    void *ret = (void *)emi_retdata_alloc(msg, size);
    if(ret == NULL){
        return -1;
    }

    memcpy(ret, data, size);

    return 0;
}


/*
 *emi_init must be used before recieving process.it uses the emi_config struct, which may requre emi config file.
 *
 * */
int emi_init(){

    struct sigaction act, old_act;
    act.sa_handler=(void(*)(int))func_sterotype;
    sigfillset(&act.sa_mask);
    act.sa_flags= SA_RESTART;    //this is important, RESTART system call during signal handler.
    if(sigaction(SIGUSR2,&act,&old_act)){
        return -1;
    }

    eu32 pid_max = get_pid_max();

    __recv_pid=getpid();

    if((emi_global.shm_id=emi_shm_init("emilib", GET_SHM_SIZE(pid_max), 0))<0){
        return -1;
    }

    void *base;
    if((base=(void *)emi_shm_alloc(emi_global.shm_id, EMI_SHM_READ|EMI_SHM_WRITE))==NULL){
        emilog(EMI_ERROR, "Critical error when alloc shared area, exit\n");
        emi_shm_destroy("emilib", emi_global.shm_id);
        return -1;
    }

    emi_shmem_mgr_init(&__recv_mgr, base, pid_max);

    if((emi_thread_pool_init(&__recv_pool, 2))<0){
        emi_shm_free(base);
        emi_shm_destroy("emilib", emi_global.shm_id);
        return -1;
    }

    emi_spin_init(&__func_list_lock);

    /*
     *  One problem is sigaction take too long time to be ready.
        The sigal register function had been moved to emi_init for longer wait,
        but still not enough for signal handler to be ready
    
        eg: in test_emi_msg_send_inside_msg_hander, add sleep after emi_init
        will work normally, if not, signal handler will fail to run sometimes
        even if log shows emi_core has send the signal successfully.
        
        temp solution: add time.sleep(0.1) to emi_init for python (don't know if
        it is also a problem for c)


        the "sigaction take too long time to be ready" problem repeated in
        cpp as well. meaning it may not be python's problem but a issue from
        system. sleep after emi_init() has to be in C function, not just
        python function

     */

    struct timespec req = {0, 1000*1000*100};
    nanosleep(&req, NULL);

    return 0;
}

void emi_loop(void){
    while(1){
        pause();
    }
}

