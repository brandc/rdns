CC=gcc
DEBUG=-Wall -Wpedantic
CFLAGS=-O2

all:
	$(CC) $(CFLAGS) main.c -o rdns

debug:
	$(CC) $(DEBUG) main.c -o rdns

clean:
	rm -f rdns
