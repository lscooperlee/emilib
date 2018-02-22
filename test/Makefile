include ../config.mk

CC=$(CROSS)gcc
CPP=$(CROSS)g++

ifeq ($(strip $(DEBUG)),y)
DEBUG = -g -DDEBUG
else
DEBUG =
endif

CFLAGS = -O2 -Wall $(DEBUG) -I../include/ -I../src/
CPPFLAGS = -std=c++14 $(CFLAGS)

LDFLAGS = -L../.out/lib/ -lemi -pthread

CSRCS=$(shell find unit_test/ -name "*.c")
COBJS=$(patsubst %.c, $(BUILDPATH)%.o, $(CSRCS))
CEXES=$(patsubst %.o, $(BUILDPATH)%, $(COBJS))

CPPSRCS=$(shell find unit_test/ -name "*.cpp")
CPPOBJS=$(patsubst %.cpp, $(BUILDPATH)%.o, $(CPPSRCS))
CPPEXES=$(patsubst %.o, $(BUILDPATH)%, $(CPPOBJS))

EMITESTSRC=emi_test.cpp
EMITESTOBJ=$(EMITESTSRC:.cpp=.o)
EMITESTEXEC=$(EMITESTSRC:.cpp=)

all: $(CPPEXES) $(CEXES) $(EMITESTEXEC)

$(EMITESTEXEC): $(EMITESTSRC)
	$(CPP) $(CPPFLAGS) $? -o $@ $(LDFLAGS)

$(CEXES):%: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(CPPEXES):%: %.cpp
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(COBJS) $(CPPOBJS) $(CEXES) $(CPPEXES) $(EMITESTEXEC)
