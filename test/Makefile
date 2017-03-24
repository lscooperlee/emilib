include ../config.mk

CC=$(CROSS)gcc
CPP=$(CROSS)g++

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
CPPFLAGS = -std=c++14 $(CFLAGS)

LDFLAGS = -L../.out/lib/ -lemi -pthread

CSRCS=$(shell find . -name "*.c")
COBJS=$(patsubst %.c, $(BUILDPATH)%.o, $(CSRCS))
CEXES=$(patsubst %.o, $(BUILDPATH)%, $(COBJS))

CPPSRCS=$(shell find . -name "*.cpp")
CPPOBJS=$(patsubst %.cpp, $(BUILDPATH)%.o, $(CPPSRCS))
CPPEXES=$(patsubst %.o, $(BUILDPATH)%, $(CPPOBJS))

all: $(CEXES) $(CPPEXES)

$(CEXES):%: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(STATIC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(CPPEXES):%: %.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(STATIC) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(COBJS) $(CPPOBJS) $(CEXES) $(CPPEXES)

	
