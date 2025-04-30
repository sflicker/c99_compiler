CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = main.o tokenizer.o parser.o emitter.o

mimic99: $(OBJS)
	$(CC) $(CFLAGS) -o mimic99 $(OBJS)

main.o: main.c 

test: mimic99
	./run_all_tests.sh mimic99

clean:
	rm -f mimic99 $(OBJS) tests/*.s tests/*.o
	@for cfile in tests/test*.c; do \
		base=$${cfile%.c}; \
		if [ -f "$$base" ]; then \
			echo "Deleting binary $$base"; \
			rm -f "$$base"; \
		fi; \
	done
	