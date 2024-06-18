all: build run

build: main.c
	gcc -o metaballs main.c ll.c -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib -pthread -lSDL2 -lSDL2_gfx

run: build
	./metaballs
