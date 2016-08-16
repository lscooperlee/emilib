include ../config.mk

CC=$(CROSS)gcc

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

CFLAGS=-O2 -Wall $(DEBUG) -I../include/ -I./include
SENDERLDFLAGS=-L../.out/lib/ -lemis
RECEIVERLDFLAGS=-L../.out/lib/ -lemi -pthread


SENDERSRCS=$(shell find sender -name "*.c")
SENDEROBJS=$(patsubst %.c, $(BUILDPATH)%.o, $(SENDERSRCS))
SENDEREXES=$(patsubst %.o, $(BUILDPATH)%, $(SENDEROBJS))

RECEIVERSRCS=$(shell find receiver -name "*.c")
RECEIVEROBJS=$(patsubst %.c, $(BUILDPATH)%.o, $(RECEIVERSRCS))
RECEIVEREXES=$(patsubst %.o, $(BUILDPATH)%, $(RECEIVEROBJS))

all: $(SENDEREXES) $(RECEIVEREXES)

$(SENDEREXES):%: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(SENDERLDFLAGS)

$(RECEIVEREXES):%: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(RECEIVERLDFLAGS)

clean:
	rm -rf $(SENDEROBJS) $(SENDEREXES) $(RECEIVEROBJS) $(RECEIVEREXES)

	
