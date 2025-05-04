#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_DIV,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LE,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_GT,
    TOKEN_BANG,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char * text;
    int length;
    int int_value;
} Token;

typedef struct {
    Token * data;
    int capacity;
    int count;
} TokenList;

const char * token_type_name(TokenType type);

#endif
