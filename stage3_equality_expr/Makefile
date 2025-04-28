CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = tutor3.o

tutor3: $(OBJS)
	$(CC) $(CFLAGS) -o tutor3 $(OBJS)

tutor3.o: tutor3.c 

test: tutor3
	./run_all_tests.sh tutor3

clean:
	rm -f tutor3 $(OBJS) tests/*.s tests/*.o
	@for cfile in tests/test*.c; do \
		base=$${cfile%.c}; \
		if [ -f "$$base" ]; then \
			echo "Deleting binary $$base"; \
			rm -f "$$base"; \
		fi; \
	done
	