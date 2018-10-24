DEBUG ?= n #y/n
STATIC ?= n
SAN ?=
CC ?= gcc
CXX ?= g++

#####################################

SHMEM ?= POSIX_SHMEM # POSIX_SHMEM/SYSV_SHMEM/FILE_SHMEM
EMI_ORDER_NUM ?= 16

#####################################

ifeq ($(strip $(STATIC)),y)
STATICFLAG = -static
else
STATICFLAG =
endif

ifeq ($(strip $(DEBUG)),y)
DEBUG = -g -DDEBUG
else
DEBUG = 
endif

FLAGS = $(DEBUG) -O2 -Wextra -Wall -D$(SHMEM) -DEMI_ORDER_NUM=$(EMI_ORDER_NUM) -D_POSIX_C_SOURCE=200809
CFLAGS ?= $(FLAGS) -std=c11
CPPFLAGS ?= $(FLAGS) -std=c++17

LDFLAGS ?= -lemi

ifdef SAN
CFLAGS += -fsanitize=$(SAN)
CPPFLAGS += -fsanitize=$(SAN)
LDFLAGS += -fsanitize=$(SAN)
endif
