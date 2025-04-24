CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = tutor1.o

tutor1: $(OBJS)
	$(CC) $(CFLAGS) -o tutor1 $(OBJS)

tutor1.o: tutor1.c 

clean:
	rm -f tutor1 $(OBJS)
	