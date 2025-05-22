CC = gcc
CFLAGS = -Wall -g -std=c99
SRC = main.c tokenizer.c parser.c emitter.c symtab.c util.c runtime_info_decorator.c ast_printer.c tokenizer_context.c parser_util.c
OBJS = $(SRC:.c=.o)

mimic99: $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o mimic99

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
	