CC = gcc
CFLAGS = -O0 -Wall -g -lpthread
src=$(wildcard *.c)  
obj=$(patsubst %.c,%.o,$(src))	


work:../src/main.c ../src/thread_pool.c ../src/copy.c ../src/process.c
	$(CC) $^ -o $@ $(CFLAGS)

debug:../src/main.c ../src/thread_pool.c ../src/copy.c ../src/process.c
	$(CC) $^ -o $@ $(CFLAGS) -DDEBUG

clean:
	$(RM) .*.sw? test debug *.o

.PHONY:all clean
