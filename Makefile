CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined -Iinclude -DDEBUG

all: build/words

build:
	mkdir -p build

build/words.o: src/words.c include/words.h | build
	$(CC) $(CFLAGS) -c src/words.c -o build/words.o

build/words: build/words.o
	$(CC) $(CFLAGS) build/words.o -o build/words

clean:
	rm -rf build

.PHONY: all 