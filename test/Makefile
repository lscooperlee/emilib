include ../config.mk

CC=$(CROSS)gcc

ifeq ($(strip $(STATIC)),y)
STATIC = -static
endif

ifeq ($(strip $(DEBUG)),y)
DEBUG = -g -DDEBUG
endif

CFLAGS=-O2 -Wall $(DEBUG) -I../include/ 

LDFLAGS=-L../.out/lib/ -lemi -pthread


SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst ./%.c, $(BUILDPATH)%.o, $(SRCS))
EXES=$(patsubst %.o, $(BUILDPATH)%, $(OBJS))

all: $(EXES)

$(BUILDPATH)%: %.c
	@mkdir -p $(dir $@)
	$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(LDFLAGS)


clean:
	rm -rf $(OBJS) $(EXES)
	
