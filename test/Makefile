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

CFLAGS=-O2 -Wall $(DEBUG) -I../include/ 

LDFLAGS=-L../.out/lib/ -lemir -lemis -pthread


SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst ./%.c, $(BUILDPATH)%.o, $(SRCS))
EXES=$(patsubst %.o, $(BUILDPATH)%, $(OBJS))

all: $(EXES)

$(BUILDPATH)%: %.c
	@mkdir -p $(dir $@)
	@echo CC	$<
	@$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(LDFLAGS)


clean:
	rm -rf $(OBJS) $(EXES)
	
