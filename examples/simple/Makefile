include ../../config.mk

CPPFLAGS += -I../../include
LDFLAGS += -L../../.out/lib/

ifeq ($(strip $(STATIC)),y)
LDFLAGS += -pthread
ifeq ($(strip $(SHMEM)),POSIX_SHMEM)
LDFLAGS += -lrt
endif
endif

all: cppsender cppreceiver

cppsender: cppsender.cpp
	@echo CXX $@
	@$(CXX) $(STATICFLAG) $(CPPFLAGS) -o $@ $? $(LDFLAGS)

cppreceiver: cppreceiver.cpp
	@echo CXX $@
	@$(CXX) $(STATICFLAG) $(CPPFLAGS) -o $@ $? $(LDFLAGS)

clean:
	rm -f cppsender cppreceiver
