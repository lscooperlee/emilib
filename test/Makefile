
CC=$(CROSS)gcc

CFLAGS=-I../include/ -g

LDFLAGS= -L../lib/ -lemi

ifneq ($(DEBUG),)
CFLAGS+=$(DEBUG)
endif

all:autotest_receiver autotest_sender sar

autotest_receiver:autotest_receiver.c
	$(CC) $(CFLAGS) -static -o $@ $< $(LDFLAGS)
autotest_sender:autotest_sender.c
	$(CC) $(CFLAGS) -static -o $@ $< $(LDFLAGS)
sar:sar.c
	$(CC) $(CFLAGS) -static -o $@ $< $(LDFLAGS)

clean:
	rm autotest_sender autotest_receiver sar -f
