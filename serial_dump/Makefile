
.PHONY: all test clean

all: serialdump

LDFLAGS=-Wall

serialdump: serialdump.c
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm serialdump
