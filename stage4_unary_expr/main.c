#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "emitter.h"
#include "parser.h"
#include "token.h"
#include "tokenizer.h"

/* 
   Simple C compiler example 

   Simplifed grammar

   <program>       ::= <function_decl>

   <function_decl> ::= "int" <identifier> "(" ")" "{" <statement> "}"

   <statement>     ::= "return" <expression> ";"

   <expression> ::= <equality_expr>

   <equality_expr> ::= <relational_expr> ( ("==" | "!=") <relational_expr> )*

   <relational_expr> ::= <additive_expr> ( ( "<" | "<=" | ">" | ">=") <additive_expr> )*

   <additive_expr> ::= <term> { ("+" | "-") <term> }*

   <term>       ::= <factor> { ("*" | "/") <factor> }*
   
   <factor>     ::= <int_literal> | "(" <expression> ")"

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*

    <int_literal> ::= [0-9]+
*/

void cleanup_token_list(TokenList * tokenList) {
    for (int i=0;i<tokenList->count;i++) {
        free((void*)tokenList->data[i].text);
    }
}

void formatted_output(const char * label, const char * text, TokenType tokenType) {
    char left[64];
    char right[64];
    snprintf(left, sizeof(left), "%s: %s", label, text);
    snprintf(right, sizeof(right), "TOKEN_TYPE: %s", token_type_name(tokenType));
    printf("%-25s %-25s\n", left, right);
}

/* Parser functions */


void print_ast(ASTNode * node, int indent) {
    if(!node) return;

    for (int i=0;i<indent;i++) printf("  ");

    switch(node->type) {
        case AST_PROGRAM:
            printf("ProgramDecl:\n");
            print_ast(node->program.function, 1);
            break;
        case AST_FUNCTION:
            printf("FunctionDecl: %s\n", node->function.name);
            print_ast(node->function.body, indent+1);
            break;
        case AST_RETURN_STMT:
            printf("ReturnStmt:\n");
            print_ast(node->return_stmt.expr, indent+1);
            break;
        case AST_INT_LITERAL:
            printf("IntLiteral: %d\n", node->int_value);
            break;
        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", token_type_name(node->binary_op.op));
            print_ast(node->binary_op.lhs, indent+1);
            print_ast(node->binary_op.rhs, indent+1);
            break;
        default:
            printf("Unknown AST Node Type\n");
            break;
    }
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

    const char * sample_program = read_text_file(program_file);

    TokenList tokenList;

    printf("Compiling\n\n%s\n\n", sample_program);

    tokenize(sample_program, &tokenList);

    // output list
    for (int i=0;i<tokenList.count;i++) {
        Token token = tokenList.data[i];
        formatted_output("TOKEN:", token.text, token.type);
    }

    ParserContext parserContext;
    parserContext.list = &tokenList;
    parserContext.pos = 0;

    ASTNode * program = parse_program(&parserContext);
    print_ast(program, 0);

    codegen(program, output_file);

    cleanup_token_list(&tokenList);
    
}                             