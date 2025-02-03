.PHONY: main ppm

INCLUDE=-Iinclude/SDL2
LIB=-Llib/SDL2
CFLAGS=-Wall -g -Wextra -std=c11 -pedantic
LFLAGS=-lSDL2

all: main

main: main.c
	clang $(CLAGS) $(LFLAGS) $(INCLUDE) $(LIB) main.c -o main
