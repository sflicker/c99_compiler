
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "file_handler.h"
#include "preprocessor.h"
#include "tokenizer.h"

#define MAX_FILES 4
#define MAX_TOKENS 256


typedef struct {
    const char* filename;
    const char* content;
    const char* cursor;
    int line;
    int column;
} FileContext;

FileContext file_stack[MAX_FILES];
Token tokens[MAX_TOKENS];
int token_count = 0;
int file_top = -1;

void push_file(const char* filename) {
    const char* contents = load_file(filename);
    if (!contents) {
        fprintf(stderr, "File not found: %s\n", filename);
    }
    file_top++;
    file_stack[file_top] = (FileContext) {
        .filename = filename,
        .content = contents,
        .cursor = contents,
        .line = 1,
        .column = 1
    };
}

void pop_file() {
    if (file_top >= 0) file_top--;
}

#define current_file (&file_stack[file_top])




void tokenize_file() {

}

int main() {
    // const char* source_code =
    // "int main() {\n"
    // "   int x = 42;\n"
    // "   printf(\"x = %d\\n\", x);\n"
    // "   return 0;\n"
    // "}\n";

    // char preprocessed[4096];
    // printf("Original: \n%s\n", source_code);
    // preprocess(source_code, preprocessed, sizeof(preprocessed));
    // printf("preprocessed: \n%s\n", preprocessed);

    // tokenize(preprocessed);


    push_file("main.c");
    tokenize_file();


    return 0;
}