#ifndef TOKENIZER_H
#define TOKENIZER_H


typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_EQ,           // ==
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_INCREMENT,    // ++
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_LE,           // <=
    // ...
} TokenType;

typedef struct {
    TokenType type;
    const char * filename;
    int line;
    int column;
} Token;

void tokenize(const char* code);


#endif