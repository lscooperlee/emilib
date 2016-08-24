#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "emi_shmem.h"
#include "emi_config.h"
#include "emi_dbg.h"

int emi_shm_init(char *name, size_t size, int mode){
    int shm_id;
    char pathname[PATH_MAX] = "/dev/shm/";
    strcat(pathname, name);

    creat(pathname, 0);
    key_t key = ftok(pathname,0xCC);

    mode = mode == EMI_SHM_CREATE ? IPC_CREAT|IPC_EXCL|0666 : 0; 
    if((shm_id=shmget(key, size, mode))<0){
        return -1;
    }
    return shm_id;
}

void *emi_shm_alloc(int id, int flag){
    void *p = NULL;

    flag = flag == (EMI_SHM_READ|EMI_SHM_WRITE) ? 0 : flag;
    if((p=(void *)shmat(id,(const void *)0,flag))==(void *)-1){
        return NULL;
    }
    return p;
}

int emi_shm_free(void *addr){
    return shmdt(addr);
}

int emi_shm_destroy(int id){
    return shmctl(id,IPC_RMID,NULL);
}


#ifdef POSIX_SHMEM

static size_t shmem_size = 0;

int emi_shm_init(char *name){
    int shm_id;
    //if((shm_id = shm_open(name, int oflag, mode_t mode))<0){
    //}
    return shm_id;
}

#endif

