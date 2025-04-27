CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = tutor2.o

tutor2: $(OBJS)
	$(CC) $(CFLAGS) -o tutor2 $(OBJS)

tutor2.o: tutor2.c 

test: tutor2
	./run_all_tests.sh tutor2

clean:
	rm -f tutor2 $(OBJS) tests/*.s tests/*.o
	@for cfile in tests/test*.c; do \
		base=$${cfile%.c}; \
		if [ -f "$$base" ]; then \
			echo "Deleting binary $$base"; \
			rm -f "$$base"; \
		fi; \
	done
	