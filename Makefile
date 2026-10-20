CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined -Iinclude -DDEBUG

all: build/words

build:
	mkdir -p build

build/words.o: src/words.c include/words.h | build
	$(CC) $(CFLAGS) -c src/words.c -o build/words.o

build/words: build/words.o
	$(CC) $(CFLAGS) build/words.o -o build/words

robust-setup:
	chmod 000 test/robust/locked.txt test/robust/subdir/noread.txt test/robust/subdir/closed  

robust-cleanup:
	chmod 755 test/robust/subdir/closed
	chmod 644 test/robust/locked.txt test/robust/subdir/noread.txt

clean:
	rm -rf build

.PHONY: all 