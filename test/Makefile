include ../config.mk

ifeq ($(strip $(DEBUG)),y)
DEBUG = -g -DDEBUG
else
DEBUG =
endif

CFLAGS = -O2 -Wall $(DEBUG) -I../include/ -I../src/
CPPFLAGS = -std=c++17 $(CFLAGS)

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

EMIUNITTESTSRC=emi_unit_test.cpp
EMIUNITTESTOBJ=$(EMIUNITTESTSRC:.cpp=.o)
EMIUNITTESTEXEC=$(EMIUNITTESTSRC:.cpp=)

all: $(EMIUNITTESTEXEC) $(EMITESTEXEC)

$(EMITESTEXEC): $(EMITESTSRC)
	@echo LD 	$@
	@$(CXX) $(CPPFLAGS) $? -o $@ $(LDFLAGS)

$(EMIUNITTESTEXEC): $(EMIUNITTESTOBJ) $(CPPOBJS)
	@echo LD 	$@
	@$(CXX) $(CPPFLAGS) $(EMIUNITTESTOBJ) $(CPPOBJS) -o $@ $(LDFLAGS)

$(CEXES):%: %.c
	@echo CC 	$@
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS) $(EMIUNITTESTOBJ):%.o: %.cpp
	@echo CXX 	$@
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(COBJS) $(CPPOBJS) $(CEXES) $(CPPEXES) $(EMITESTEXEC) $(EMIUNITTESTEXEC) $(EMIUNITTESTOBJ)
