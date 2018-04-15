include ../config.mk

CFLAGS += -I../include/ -I../src/
CPPFLAGS += -I../include/ -I../src/

LDFLAGS += -L../.out/lib/ -pthread
ifeq ($(strip $(SHMEM)),POSIX_SHMEM)
#LDFLAGS += -lrt
endif

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

EMISTRESSTESTSRCS=$(shell find stress_test/ -name "*.cpp")
EMISTRESSTESTOBJS=$(patsubst %.cpp, $(BUILDPATH)%.o, $(EMISTRESSTESTSRCS))
EMISTRESSTESTEXES=$(patsubst %.o, $(BUILDPATH)%, $(EMISTRESSTESTOBJS))

all: $(EMITESTEXEC) $(EMIUNITTESTEXEC) $(EMISTRESSTESTEXES)

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

$(EMISTRESSTESTEXES):%: %.cpp
	@echo CXX 	$@
	@$(CXX) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

clean:
	@rm -rf $(COBJS) $(CPPOBJS) $(CEXES) $(CPPEXES) $(EMITESTEXEC) $(EMIUNITTESTEXEC) $(EMIUNITTESTOBJ) $(EMISTRESSTESTEXES)
