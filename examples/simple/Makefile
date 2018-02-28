
CFLAGS = -std=c++17 -O2 -Wextra -Wall -I../../include
LDFLAGS = -L../../.out/lib/ -lemi 

all: cppsender cppreceiver

cppsender: cppsender.cpp
	@echo CXX $@
	@$(CXX) $(CFLAGS) -o $@ $? $(LDFLAGS)

cppreceiver: cppreceiver.cpp
	@echo CXX $@
	@$(CXX) $(CFLAGS) -o $@ $? $(LDFLAGS)

clean:
	rm -f cppsender cppreceiver
