#ifndef TOKEN_H
#define TOKEN_H

#include "list_util.h"

typedef enum {
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_SHORT,
    TOKEN_LONG,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_GOTO,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_SWITCH,
    TOKEN_ASSERT_EXTENSION,
    TOKEN_PRINT_EXTENSION,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_DIV,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_COMMA,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LE,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_GT,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_PLUS_EQUAL,
    TOKEN_MINUS_EQUAL,
    TOKEN_BANG,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char * text;
    int length;
    int int_value;
    int line;
    int col;
} Token;

// typedef struct {
//     Token * data;
//     int capacity;
//     int count;
// } TokenList;

// typedef struct TokenData {
//     Token * token;
//     struct TokenData * next;
// } TokenData;

// typedef struct TokenList {
//     TokenData * head;
//     TokenData * tail;
// } TokenList;

DEFINE_LINKED_LIST(Token*, tokenlist);

const char * token_type_name(TokenType type);

void init_token_list(tokenlist * list);
void cleanup_token_list(tokenlist * tokenList);
void add_token(tokenlist * list, Token * token);
Token * make_token(TokenType type, const char * text, int line, int col);

#endif
