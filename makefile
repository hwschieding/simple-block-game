main: main.c
	gcc -o game.exe main.c -I include -L lib -lraylib -lgdi32 -lwinmm
