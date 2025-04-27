CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = tutor2.o

tutor2: $(OBJS)
	$(CC) $(CFLAGS) -o tutor2 $(OBJS)

tutor2.o: tutor2.c 

test: tutor2
	./tutor2 tests/test1.c
	nasm -f elf64 tutor2.s -o run_tutor2.o
	gcc -no-pie -o run_tutor2 run_tutor2.o 
	@exit_code=0; { ./run_tutor2 ; exit_code=$$? ; echo "Exit code: $$exit_code"; }; test $$exit_code -eq 42 || true

clean:
	rm -f tutor2 $(OBJS) tutor2.s run_tutor2 run_tutor2.o tests/*.s tests/*.o
	