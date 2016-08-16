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
LIBSENDER = libemis.so
LIBRECEIVER = libemir.so

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

LDFLAGS = -L$(LIBDIR) -lemir -lemis -lpthread
LIBLDFLAGS = -shared


LIBSENDERSRCS=src/emiif.c src/emi_sock.c
LIBSENDEROBJS=$(patsubst %,$(TMPDIR)/%,$(LIBSENDERSRCS:.c=.o))

LIBRECEIVERSRCS=src/emi_config.c src/emi_dbg.c src/emi_shmem.c src/emi_core.c
LIBRECEIVEROBJS=$(patsubst %,$(TMPDIR)/%,$(LIBRECEIVERSRCS:.c=.o))

CORESRCS=src/main.c
COREOBJS=$(patsubst %,$(TMPDIR)/%,$(CORESRCS:.c=.o))

SARSRCS=src/emi_sar.c
SAROBJS=$(patsubst %,$(TMPDIR)/%,$(SARSRCS:.c=.o))

.PHONY:all clean

all:$(LIBSENDER) $(LIBRECEIVER) $(CORE) $(SAR) TEST

$(LIBSENDER):$(LIBSENDEROBJS)
	@echo LD		$(LIBSENDER)
	@$(MKDIR) $(LIBDIR)
	@$(CC) $(LIBLDFLAGS) -o $(LIBDIR)/$@ $?

$(LIBRECEIVER):$(LIBRECEIVEROBJS)
	@echo AR		$(LIBRECEIVER)
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

$(LIBSENDEROBJS):$(TMPDIR)/%.o:%.c
	@echo CC		$^
	@$(MKDIR) `$(DIRNAME) $@`
	@$(CC) $(LIBCFLAGS) -c -o $@ $<

$(LIBRECEIVEROBJS):$(TMPDIR)/%.o:%.c
	@echo CC		$^
	@$(MKDIR) `$(DIRNAME) $@`
	@$(CC) $(LIBCFLAGS) -c -o $@ $<

TEST:
	@make -C $(TEST)

clean:
	$(RM) $(TMPDIR)
	@make -C $(TEST) clean

tests_run:
	@python3 test/test.py
