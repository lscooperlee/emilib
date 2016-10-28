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

CFLAGS = -O2 -Wall $(DEBUG) -I../include/ -I./include -DEMI_ORDER_NUM=$(EMI_ORDER_NUM)
LDFLAGS = -L../.out/lib/ -lemi -pthread

SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %.c, $(BUILDPATH)%.o, $(SRCS))
EXES=$(patsubst %.o, $(BUILDPATH)%, $(OBJS))

all: $(EXES)

$(EXES):%: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(EXES)

	
