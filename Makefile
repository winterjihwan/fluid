.PHONY: main

CXX=clang
INCLUDE=-Iinclude/SDL2 fluid.c
LIB=-Llib/SDL2
CFLAGS=-Wall -g -Wextra -std=c11 -pedantic
LFLAGS=-lSDL2

all: main

main: main.c fluid.c
	$(CXX) $(CLAGS) $(LFLAGS) $(INCLUDE) $(LIB) main.c -o main
