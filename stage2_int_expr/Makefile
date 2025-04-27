CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = tutor2.o

tutor2: $(OBJS)
	$(CC) $(CFLAGS) -o tutor2 $(OBJS)

tutor2.o: tutor2.c 

test: tutor2
	./run_all_tests.sh tutor2

clean:
	rm -f tutor2 $(OBJS) tutor2.s run_tutor2 run_tutor2.o tests/*.s tests/*.o
	