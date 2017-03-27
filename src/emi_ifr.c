
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

#include "emi_ifr.h"
#include "emi_msg.h"
#include "emi_sock.h"
#include "emi_shbuf.h"
#include "list.h"
#include "emi_semaphore.h"
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

LIST_HEAD(__func_list);

static struct emi_thread_pool __recv_pool;
static struct emi_shmem_mgr __recv_mgr;
static eu32 __recv_pid;

void __func_sterotype(void *no_use);
void func_sterotype(int no_use){
    emi_thread_pool_submit(&__recv_pool, __func_sterotype, NULL);
}

void __func_sterotype(void *no_use){
    int nth;
    struct emi_msg *shmsg;
    struct list_head *lh;

    nth=__recv_mgr.pididx[__recv_pid];

    shmsg = (struct emi_msg *)get_shbuf_addr(__recv_mgr.base, nth);

    //this address is the pid_num in emi_core.it should be changed as soon as possable.emi_core will wait the zero to make sure the this process recieved the signal ,and send the same signal to other process that registered the same massage.
    __recv_mgr.pididx[__recv_pid] = 0;

    //For the possible user retdata allocation
    //espinlock_t *lock = GET_MSGBUF_LOCK_BASE(base);
    update_emi_buf_lock(__recv_mgr.base, __recv_mgr.msgbuf, __recv_mgr.msgbuf_lock);

    int ret=-1;
    list_for_each(lh,&__func_list){
        struct func_list *fl;
        fl=container_of(lh,struct func_list,list);
        if(fl->msg==shmsg->msg){
            emilog(EMI_DEBUG, "Registerd function with msg num %d founded\n", shmsg->msg);

            ret=(fl->func)(shmsg);

            emilog(EMI_DEBUG, "Registerd function with msg num %d done \n", shmsg->msg);

            //One function for one message in one process
            break;
        }
    }

    emilog(EMI_DEBUG, "Handler for msg %d done, %d returned\n", shmsg->msg, ret);
    emi_spin_lock(&shmsg->lock);
    emilog(EMI_DEBUG, "lock aquired");

    //Don't need lock here, no one changes this bit.
    if(shmsg->flag&EMI_MSG_MODE_BLOCK){

        if(ret){
            emilog(EMI_DEBUG, "msg handler running failed (not return zero)");
            shmsg->flag&=~EMI_MSG_RET_SUCCEEDED;
        }
        
    }
    
    shmsg->count--;
    emi_spin_unlock(&shmsg->lock);
    emilog(EMI_DEBUG, "lock released");

    return;
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

    ret = 0;

    emilog(EMI_INFO, "Emi register done");

out:
    emi_close(sd);
    return ret;
}

int emi_msg_register(eu32 defined_msg,emi_func func){
    return __emi_msg_register(defined_msg,func, 0);
}

char *emi_retdata_alloc(const struct emi_msg *cmsg, eu32 size){
    struct emi_msg *msg = (struct emi_msg *)cmsg;

    emi_spin_lock(&msg->lock);

    if(msg->flag & EMI_MSG_FLAG_RETDATA){
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        emi_spin_unlock(&msg->lock);
        return NULL;
    }

    if (!(msg->flag & EMI_MSG_MODE_BLOCK)) {
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        emi_spin_unlock(&msg->lock);
        return NULL;
    }

    void *addr = emi_alloc(size);
    if(addr == NULL){
        msg->flag &= ~EMI_MSG_RET_SUCCEEDED;
        emi_spin_unlock(&msg->lock);
        return NULL;
    }

    msg->retdata_offset = GET_OFFSET(msg, addr);

    msg->retsize = size;
    msg->flag |= EMI_MSG_FLAG_RETDATA;

    emi_spin_unlock(&msg->lock);
    
    return addr;
}

int emi_msg_prepare_return_data(const struct emi_msg *msg, void *data, eu32 size) {
    
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

    struct sigaction act, old_act;
    act.sa_handler=func_sterotype;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask,SIGUSR2);
//    act.sa_flags=SA_INTERRUPT;
    act.sa_flags= SA_RESTART;    //this is important, RESTART system call during signal handler.
    if(sigaction(SIGUSR2,&act,&old_act)){
    //    goto out;
        return -1;
    }

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

    usleep(100000);

    return 0;
}

void emi_loop(void){
    while(1){
        pause();
    }
}

