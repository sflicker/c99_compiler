#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "ast.h"
#include "emitter.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"





void formatted_output(const char * label, const char * text, TokenType tokenType) {
    char left[64];
    char right[64];
    snprintf(left, sizeof(left), "%s: %s", label, text);
    snprintf(right, sizeof(right), "TOKEN_TYPE: %s", token_type_name(tokenType));
    printf("%-25s %-25s\n", left, right);
}

/* main and related */

const char * read_text_file(const char* filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char * buffer = malloc(filesize + 1);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        exit(1);
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if (read_size != filesize) {
        fprintf(stderr, "fread failed\n");
        fclose(file);
        free(buffer);
        exit(1);
    }

    buffer[filesize] = '\0';
    fclose(file);
    return buffer;

}

char* change_extension(const char* source_file, const char* new_ext) {
    const char* dot = strrchr(source_file, '.'); // find last dot
    size_t base_length = (dot) ? (size_t)(dot - source_file) : strlen(source_file);

    size_t new_length = base_length + strlen(new_ext);
    char* out_file = malloc(new_length + 1); // +1 for '\0'
    if (!out_file) {
        perror("malloc");
        exit(1);
    }

    memcpy(out_file, source_file, base_length);
    strcpy(out_file + base_length, new_ext);

    return out_file;
}

int main(int argc, char ** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file>", argv[0]);
        return 1;
    }
    const char * program_file = argv[1];
    const char * output_file = change_extension(program_file, ".s");

    const char * program_text = read_text_file(program_file);

    TokenList tokenList;

    printf("Compiling\n\n%s\n\n", program_text);

    tokenize(program_text, &tokenList);

    // output list
    for (int i=0;i<tokenList.count;i++) {
        Token token = tokenList.data[i];
        formatted_output("TOKEN:", token.text, token.type);
    }

    ParserContext parserContext;
    initialize_parser(&parserContext, &tokenList);

    ASTNode * translation_unit = parse_translation_unit(&parserContext);
    print_ast(translation_unit, 0);

    emit_translation_unit(translation_unit, output_file);

    cleanup_token_list(&tokenList);
    
}                             