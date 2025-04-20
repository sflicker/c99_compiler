CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS = main.o tokenizer.o preprocessor.o keywords.o macros.o file_handler.o

driver: $(OBJS)
	$(CC) $(CFLAGS) -o driver $(OBJS)

main.o: main.c preprocessor.h tokenizer.h file_handler.h
tokenizer.o: tokenizer.c tokenizer.h preprocessor.h keywords.h
preprocessor.o: preprocessor.c preprocessor.h macros.h file_handler.h
macros.o: macros.c macros.h
keywords.o: keywords.c keywords.h
file_handler.o: file_handler.c file_handler.o

clean:
	rm -f driver $(OBJS)
	