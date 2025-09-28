CC = gcc

editor : src/main.c
	$(CC) src/main.c -o build/main -Wall -Wextra -O2 -pedantic -std=c99 

run : 
	cd build && ./main
