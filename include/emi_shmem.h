#ifndef __SHMEM_H__
#define __SHMEM_H__

#include "emi_config.h"
#include "emi_shbuf.h"
#include "emi_lock.h"

#define EMI_SHM_CREATE 0x00000001

#define EMI_SHM_READ 0x00000010
#define EMI_SHM_WRITE 0x00000020

extern int emi_shm_init(const char *name, size_t size, int mode);
extern void *emi_shm_alloc(int id, int flag);
extern int emi_shm_free(void *addr);
extern int emi_shm_destroy(const char *name, int id);

#define MSG_SHM_SIZE                (BUDDY_SIZE << EMI_ORDER_NUM)
#define MSGBUF_SHM_SIZE             (2 * (sizeof(struct emi_buf)<<EMI_ORDER_NUM))
#define MSGBUF_LOCK_SHM_SIZE        (sizeof(espinlock_t))
#define PIDIDX_SHM_SIZE(pid_max)    ((pid_max) * sizeof(eu32))
#define PIDLOCK_SHM_SIZE(pid_max)    ((pid_max) * sizeof(elock_t))


#define GET_MSG_BASE(addr)                  (void *)(addr)
#define GET_MSGBUF_BASE(addr)               (void *)((char *)GET_MSG_BASE(addr) + MSG_SHM_SIZE)
#define GET_MSGBUF_LOCK_BASE(addr)          (espinlock_t *)((char *)GET_MSGBUF_BASE(addr) + MSGBUF_SHM_SIZE)
#define GET_PIDIDX_BASE(addr)               (eu32 *)((char *)GET_MSGBUF_LOCK_BASE(addr)  + MSGBUF_LOCK_SHM_SIZE)
#define GET_PIDLOCK_BASE(addr, pid_max)     (elock_t *)((char *)GET_PIDIDX_BASE(addr) + PIDIDX_SHM_SIZE(pid_max))

#define GET_SHM_SIZE(pid_max)       (MSG_SHM_SIZE + MSGBUF_SHM_SIZE + MSGBUF_LOCK_SHM_SIZE + \
                                        PIDIDX_SHM_SIZE(pid_max) + PIDLOCK_SHM_SIZE(pid_max))


struct emi_shmem_mgr {
    void *base;
    void *msgbuf;
    espinlock_t *msgbuf_lock;
    eu32 *pididx;
    elock_t *pididx_lock;
};

static inline void emi_shmem_mgr_init(struct emi_shmem_mgr *mgr, void *base, eu32 pid_max){
    mgr->base = GET_MSG_BASE(base);
    mgr->msgbuf = GET_MSGBUF_BASE(base);
    mgr->msgbuf_lock = GET_MSGBUF_LOCK_BASE(base);
    mgr->pididx = GET_PIDIDX_BASE(base);
    mgr->pididx_lock = GET_PIDLOCK_BASE(base, pid_max);
}

#endif
