include ./config.mk

CHECK = cppcheck

MKDIR = mkdir -p
DIRNAME = dirname
RM = rm -rf

#########################

TMPDIR = ./.out
LIBDIR = $(TMPDIR)/lib
BINDIR = $(TMPDIR)/bin

CORE = emi_core
SAR = emi_sar
LIBSENDER = libemis.so 		#libemi only for sender
LIBEMI = libemi.so 	#libemi for all

#########################

CHECKFLAGS = --enable=all --language=c++ -I include -I src --suppress=missingIncludeSystem --suppressions-list=suppressions.txt --inconclusive -q --std=c++14

CFLAGS += -I./include
LIBCFLAGS = $(CFLAGS) -fpic

LDFLAGS += -L$(LIBDIR)
LIBLDFLAGS = -lpthread

ifeq ($(strip $(SHMEM)),POSIX_SHMEM)
LIBLDFLAGS += -lrt
endif

LIBSENDERSRCS=src/emi_if.c src/emi_sock.c src/emi_dbg.c src/emi_config.c
LIBSENDEROBJS=$(patsubst %,$(TMPDIR)/%,$(LIBSENDERSRCS:.c=.o))

LIBRECEIVERSRCS=src/emi_ifr.c src/emi_sockr.c src/emi_config.c src/emi_dbg.c  \
				src/emi_shbuf.c src/emi_shmem.c src/emi_core.c src/emi_thread.c \
				src/emi_hash.c 
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

all:$(LIBSENDER) $(LIBEMI) $(CORE) $(SAR) PYTHON TEST EXAMPLE

$(LIBSENDER):$(LIBSENDEROBJS)
	@echo LD		$(LIBSENDER)
	@$(MKDIR) $(LIBDIR)
	@$(CC) -shared -o $(LIBDIR)/$@ $?

$(LIBEMI):$(LIBEMIOBJS)
	@echo LD		$(LIBEMI)
	@$(MKDIR) $(LIBDIR)
	@$(CC) -shared -o $(LIBDIR)/$@ $? $(LIBLDFLAGS)

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

PYTHON:
	@true

TEST:
	@make -C test/

EXAMPLE:
	@make -C examples/simple/

check:
	@$(CHECK) $(CHECKFLAGS) .

clean:
	$(RM) $(TMPDIR)
	@make -C test clean
	@make -C examples/simple clean

emi_test: 
	@./test/emi_test

python_test: 
	@python3 -W ignore test/python_test.py -v

unit_test:
	@./test/emi_unit_test

stress_test:
	@./tools/stress_test.sh

install:
	install -d /usr/include/emi/
	install -Dm755 .out/bin/* /usr/bin/
	install -Dm755 .out/lib/* /usr/lib/
	install -Dm644 include/* /usr/include/emi/
	cd python && python3 setup.py build -b ../.out/build install
