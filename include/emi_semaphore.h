#ifndef __EMI_SEMAPHORE_H__
#define __EMI_SEMAPHORE_H__

#include "emi_types.h"
#include <pthread.h>

#if 0
static inline int __emi_lock(int *p)
{
	asm volatile(
	"0:\n\t"
	"cmp $0,%0;\n\t"
//	"jle 1;\n\t"
//	"0:\n\t" 
//	"lock dec %0; \n\t"
	:"+m"(*p)
	::
	);
	return 0;
}

static inline int __emi_unlock(int *p)
{
	asm volatile(
//	"pushl %%eax; \n\t"      
	"movl %1, %%eax; \n\t"    
	"movl %%eax, %0; \n\t"
//	"popl %%eax; \n\t"
	:"+m"(*p)
	:"i"(1)
	:"%eax"
	);
	return 0;
}
#endif
//#if 0
#ifdef arm
static inline void __emi_lock(int *p)
{
	asm volatile(
	"ldr r1,[%0]\n\t"
	"0:mov r2,#0\n\t"
	"swp r3,r2,[r1]\n\t"
	"cmp r3,#1\n\t"
	"bne 0b\n\t"
	:
	:"r"(p)
	:"r1","r2","r3","memory"
	);
}
static inline void __emi_unlock(int *p){
	asm volatile(
	"mov r2,#1\n\t"
	"str r2,[%0]\n\t"
	:
	:"r"(p)
	:"r2","memory"
	);
}
#endif

#ifdef x86
static inline void __emi_lock(int *p)
{
	asm volatile(
	"pushl %%eax; \n\t" 
	"pushl %%ecx; \n\t" 
	"0:  movl $0, %%ecx; \n\t" 
	"movl $1, %%eax; \n\t" 
	"lock  cmpxchgl %%ecx, %0; \n\t"
	"jne 0b; \n\t" 
	"popl %%ecx; \n\t" 
	"popl %%eax; \n\t"
	:"+m"(*p)
	:
    :"%eax","%ecx"
	);
}

static inline void __emi_unlock(int *p)
{
	asm volatile(
//	"pushl %%eax; \n\t"      
	"movl %1, %%eax; \n\t"    
	"movl %%eax, %0; \n\t"
//	"popl %%eax; \n\t"
	:"+m"(*p)
	:"i"(1)
	:"%eax"
	);
}
#endif

#ifdef SYSTEM_LOCK
static void inline emi_lock_init(elock_t *p){
	pthread_mutex_init(p,NULL);
}

static void inline emi_lock(elock_t *p){
	pthread_mutex_lock(p);
}

static void inline emi_unlock(elock_t *p){
	pthread_mutex_unlock(p);
}
static void inline emi_lock_destroy(elock_t *p){
	return;
}

#else
static void inline emi_lock_init(elock_t *p){
	*p=1;
}
static inline emi_lock(elock_t *p){
	__emi_lock(p);
}
static inline emi_unlock(elock_t *p){
	__emi_unlock(p);
}
static void inline emi_lock_destroy(elock_t *p){
	return;
}
#endif
#endif
