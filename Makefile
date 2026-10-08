CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = 1_sigint3 alarm 3_signal_block2

all: $(TARGETS)

1_sigint3: 1_sigint3.c
	$(CC) $(CFLAGS) -o $@ $<

alarm: alarm.c
	$(CC) $(CFLAGS) -o $@ $<

3_signal_block2: 3_signal_block2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
