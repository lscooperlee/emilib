#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "emi_shmem.h"
#include "emi_config.h"

#if defined(SYSV_SHMEM)

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0')
        hash = ((hash << 5) + hash) + c;
    

    return hash;
}

int emi_shm_init(const char *name, size_t size, int mode){
    int shm_id;
    key_t key = (key_t)hash(name);

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

int emi_shm_destroy(const char *name, int id){
    return shmctl(id,IPC_RMID,NULL);
}

#else //!SYSV_SHMEM

#include <sys/mman.h>
#include <string.h>

static size_t shmem_size = 0;

#if defined(POSIX_SHMEM)

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int emi_shm_init(const char *name, size_t size, int mode){
    int shm_id;
    int _mode;
    char pathname[PATH_MAX] = "/";
    strcat(pathname, name);

    if (mode == EMI_SHM_CREATE){
        _mode = O_RDWR|O_CREAT;
    }else{
        _mode = O_RDWR;
    }

    if((shm_id = shm_open(pathname, _mode, 0666))<0){
        return -1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    size = ( (size+page_size) / page_size ) * page_size;
    shmem_size = size;

    if (mode == EMI_SHM_CREATE){
        if(ftruncate(shm_id, shmem_size)){
            shm_unlink(pathname);
            return -1;
        }
    }

    return shm_id;
}

int emi_shm_destroy(const char *name, int id){
    close(id);
    char pathname[PATH_MAX] = "/";
    strcat(pathname, name);
    shm_unlink(pathname);
    return 0;
}

#else //FILE SHMEM

int emi_shm_init(const char *name, size_t size, int mode){
    int fd;
    int _mode;
    char pathname[PATH_MAX] = "/tmp/";
    strcat(pathname, name);

    if (mode == EMI_SHM_CREATE){
        _mode = O_RDWR|O_CREAT;
    }else{
        _mode = O_RDWR;
    }

    if((fd = open(pathname, _mode, 0666))<0){
        return -1;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    size = ( (size+page_size) / page_size ) * page_size;
    shmem_size = size;

    if (mode == EMI_SHM_CREATE){
        if(ftruncate(fd, shmem_size)){
            unlink(pathname);
            return -1;
        }
    }

    return fd;
}

int emi_shm_destroy(const char *name, int id){
    close(id);
    char pathname[PATH_MAX] = "/tmp";
    strcat(pathname, name);
    unlink(pathname);
    return 0;
}

#endif //POSIX_SHMEM

void *emi_shm_alloc(int id, int flag){
    void *p = NULL;

    flag = flag == (EMI_SHM_READ|EMI_SHM_WRITE) ? PROT_READ|PROT_WRITE : PROT_READ;
    if((p = mmap(0, shmem_size, flag, MAP_SHARED, id, 0)) == MAP_FAILED){
        return NULL;
    }
    return p;
}

int emi_shm_free(void *addr){
    return munmap(addr, shmem_size);
}

#endif //SYSV_SHMEM
