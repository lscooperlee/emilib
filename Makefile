include ./config.mk

CC = $(CROSS)gcc
CPP = $(CROSS)g++
LD = $(CROSS)ld
AR = $(CROSS)ar csr
STRIP = $(CROSS)strip

MKDIR = mkdir -p
DIRNAME = dirname
RM = rm -rf

#########################

TMPDIR = ./.out
LIBDIR = $(TMPDIR)/lib
BINDIR = $(TMPDIR)/bin
TEST = test

CORE = emi_core
SAR = emi_sar
LIBSENDER = libemis.so 		#libemi only for sender
LIBEMI = libemi.so 	#libemi for all

#########################

ifeq ($(strip $(STATIC)),y)
STATIC = -static
else
STATIC =
endif

ifeq ($(strip $(DEBUG)),y)
DEBUG = -g -DDEBUG
else
DEBUG = 
endif

CFLAGS = $(DEBUG) -O2 -Wall -I./include
LIBCFLAGS = $(CFLAGS) -fpic

LDFLAGS = -L$(LIBDIR) -lemi -lpthread
LIBLDFLAGS = -shared


LIBSENDERSRCS=src/emiif.c src/emi_sock.c src/emi_dbg.c src/emi_config.c
LIBSENDEROBJS=$(patsubst %,$(TMPDIR)/%,$(LIBSENDERSRCS:.c=.o))

LIBRECEIVERSRCS=src/emi_ifr.c src/emi_config.c src/emi_dbg.c src/emi_shmem.c src/emi_core.c
LIBEMISRCS=$(sort $(LIBSENDERSRCS) $(LIBRECEIVERSRCS))
LIBEMIOBJS=$(patsubst %,$(TMPDIR)/%,$(LIBEMISRCS:.c=.o))

CORESRCS=src/main.c
COREOBJS=$(patsubst %,$(TMPDIR)/%,$(CORESRCS:.c=.o))

SARSRCS=src/emi_sar.c
SAROBJS=$(patsubst %,$(TMPDIR)/%,$(SARSRCS:.c=.o))

export LD_LIBRARY_PATH := ${PWD}/$(LIBDIR)
export PATH := ${PWD}/$(BINDIR):${PATH}
export PYTHONPATH := ${PWD}/python/emilib

.PHONY:all clean

all:$(LIBSENDER) $(LIBEMI) $(CORE) $(SAR) TEST

$(LIBSENDER):$(LIBSENDEROBJS)
	@echo LD		$(LIBSENDER)
	@$(MKDIR) $(LIBDIR)
	@$(CC) $(LIBLDFLAGS) -o $(LIBDIR)/$@ $?

$(LIBEMI):$(LIBEMIOBJS)
	@echo AR		$(LIBEMI)
	@$(MKDIR) $(LIBDIR)
	@$(CC) $(LIBLDFLAGS) -o $(LIBDIR)/$@ $?

$(CORE):$(COREOBJS)
	@echo LD		$(CORE)
	@$(MKDIR) $(BINDIR)
	@$(CC) $(STATIC) -o $(BINDIR)/$@ $? $(LDFLAGS)

$(SAR):$(SAROBJS)
	@echo LD		$(SAR)
	@$(MKDIR) $(BINDIR)
	@$(CC) $(STATIC) -o $(BINDIR)/$@ $? $(LDFLAGS)

$(COREOBJS):$(TMPDIR)/%.o:%.c
	@echo CC		$^
	@$(MKDIR) `$(DIRNAME) $@`
	@$(CC) $(CFLAGS) -c -o $@ $<

$(SAROBJS):$(TMPDIR)/%.o:%.c
	@echo CC		$^
	@$(MKDIR) `$(DIRNAME) $@`
	@$(CC) $(CFLAGS) -c -o $@ $<

$(LIBEMIOBJS):$(TMPDIR)/%.o:%.c
	@echo CC		$^
	@$(MKDIR) `$(DIRNAME) $@`
	@$(CC) $(LIBCFLAGS) -c -o $@ $<


TEST:
	@make -C $(TEST)

clean:
	$(RM) $(TMPDIR)
	@make -C $(TEST) clean

emi_test: 
	@python3 test/emi_test.py

unit_test: 
#	@python3 -W ignore test/unit_test.py 
#	@python3 -W ignore test/unit_test.py TestEmiLib.test_emi_msg_send_highlevel_unblock_senddata
	@python3 -W ignore test/unit_test.py TestEmiLib.test_emi_msg
