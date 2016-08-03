#ifndef __EMI_TYPES_H__
#define __EMI_TYPES_H__
#include <stdint.h>

/*
#ifdef X86

typedef unsigned int __eu32;
typedef signed int __es32;
typedef unsigned short __eu16;
typedef signed short __es16;
typedef unsigned char __eu8;
typedef signed char __es8;

#else

typedef unsigned int __eu32;
typedef signed int __es32;
typedef unsigned short __eu16;
typedef signed short __es16;
typedef unsigned char __eu8;
typedef signed char __es8;

#endif
*/

typedef int8_t __es8;
typedef uint8_t __eu8;
typedef int16_t __es16;
typedef uint16_t __eu16;
typedef int32_t __es32;
typedef uint32_t __eu32;

typedef __eu32 eu32;
typedef __es32 es32;
typedef __eu16 eu16;
typedef __es16 es16;
typedef __eu8 eu8;
typedef __es8 es8;

#define SYSTEM_LOCK
#ifdef SYSTEM_LOCK
typedef pthread_mutex_t elock_t;
#else
typedef es32 elock_t;
#endif

#endif
