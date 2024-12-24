all: mts

CC = gcc
CFLAGS = -Wall -std=c99 -o

mts: conductor.c file.h train.h mdata.h pqueue.h
	$(CC) $(CFLAGS) mts conductor.c -pthread

clean:
	rm -f *.o
	rm -f mts
	rm -f output.txt
