#ifndef __SHMEM_H__
#define __SHMEM_H__

#include "emi_config.h"
#include "emi_semaphore.h"

#define EMI_SHM_CREATE 0x00000001

#define EMI_SHM_READ 0x00000010
#define EMI_SHM_WRITE 0x00000020

extern int emi_shm_init(const char *name, size_t size, int mode);
extern void *emi_shm_alloc(int id, int flag);
extern int emi_shm_free(void *addr);
extern int emi_shm_destroy(const char *name, int id);

#define MSG_SHM_SIZE                (BUDDY_SIZE << EMI_ORDER_NUM)
#define EMIBUF_SHM_SIZE             (2 * (sizeof(struct emi_buf)<<EMI_ORDER_NUM))
#define EMIBUF_LOCK_SHM_SIZE        (sizeof(espinlock_t))
#define PIDIDX_SHM_SIZE(pid_max)    ((pid_max) * sizeof(eu32))
#define PIDLOCK_SHM_SIZE(pid_max)    ((pid_max) * sizeof(elock_t))


#define GET_MSG_BASE(addr)                  (addr)
#define GET_EMIBUF_BASE(addr)               (GET_MSG_BASE(addr) + MSG_SHM_SIZE)
#define GET_EMIBUF_LOCK_BASE(addr)          (GET_EMIBUF_BASE(addr) + EMIBUF_SHM_SIZE)
#define GET_PIDIDX_BASE(addr)               (GET_EMIBUF_LOCK_BASE(addr)  + EMIBUF_LOCK_SHM_SIZE)
#define GET_PIDLOCK_BASE(addr, pid_max)     (GET_PIDIDX_BASE(addr) + PIDIDX_SHM_SIZE(pid_max))

#define GET_SHM_SIZE(pid_max)       (MSG_SHM_SIZE + EMIBUF_SHM_SIZE + EMIBUF_LOCK_SHM_SIZE + \
                                        PIDIDX_SHM_SIZE(pid_max) + PIDLOCK_SHM_SIZE(pid_max))

#endif
