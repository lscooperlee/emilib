include ../../config.mk

CPPFLAGS += -I../../include
LDFLAGS += -L../../.out/lib/

all: cppsender cppreceiver

cppsender: cppsender.cpp
	@echo CXX $@
	@$(CXX) $(CPPFLAGS) -o $@ $? $(LDFLAGS)

cppreceiver: cppreceiver.cpp
	@echo CXX $@
	@$(CXX) $(CPPFLAGS) -o $@ $? $(LDFLAGS)

clean:
	rm -f cppsender cppreceiver
