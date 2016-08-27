#ifndef __SHMEM_H__
#define __SHMEM_H__

#define EMI_SHM_CREATE 0x00000001

#define EMI_SHM_READ 0x00000010
#define EMI_SHM_WRITE 0x00000020

extern int emi_shm_init(const char *name, size_t size, int mode);
extern void *emi_shm_alloc(int id, int flag);
extern int emi_shm_free(void *addr);
extern int emi_shm_destroy(const char *name, int id);

#endif
