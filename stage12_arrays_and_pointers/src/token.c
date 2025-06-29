#include <stdlib.h>
#include <string.h>

#include "list_util.h"
#include "token.h"
#include "util.h"

void init_token_list(tokenlist * list) {
    list->head = NULL;
    list->tail = NULL;
}

void add_token(tokenlist * tokens, Token * token) {
    tokenlist_append(tokens, token);
}

Token * make_token(TokenType type, const char * text, int line, int col) {
    Token * token = malloc(sizeof(Token));
    token->type = type;
    token->text = strdup(text);
    token->length = strlen(text);
    token->int_value = 0;
    token->line = line;
    token->col = col;
    return token;
}

Token * make_int_token(char * numberText, int line, int col) {
    Token * token = malloc(sizeof(Token));
    token->type = TOKEN_INT_LITERAL;
    int numberTextLen = strlen(numberText);
    char * numberTextCopy = malloc(numberTextLen + 1);
    memcpy(numberTextCopy, numberText, numberTextLen);
    numberTextCopy[numberTextLen] = '\0';
    token->text = numberTextCopy;
    token->length = numberTextLen;
    token->int_value = atoi(numberText);
    token->line = line;
    token->col = col;
    return token;
}

Token * make_identifier_token(const char * id, int line, int col) {
    Token * token = malloc(sizeof(Token));
    token->type = TOKEN_IDENTIFIER;
    token->text = strdup(id);
    token->length = strlen(id);
    token->int_value = 0;
    token->line = line;
    token->col = col;
    return token;
}

Token * make_eof_token(int line, int col) {
    Token * eofToken = malloc(sizeof(Token));
    eofToken->type = TOKEN_EOF;
    eofToken->text = '\0';
    eofToken->length = 0;
    eofToken->line = line;
    eofToken->col = col;
    return eofToken;
}

void cleanup_token_list(tokenlist * tokens) {

    for (tokenlist_node * node = tokens->head; node; node = node->next) {
        Token * token = node->value;
        free((void*)token->text);
        free(token);
    }
}

void free_token(Token * token) {
    free(token->text);
    free(token);
}

const char * token_type_name(TokenType type) {
    switch(type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INT: return "INT";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_SHORT: return "SHORT";
        case TOKEN_LONG: return "LONG";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_DO: return "DO";
        case TOKEN_GOTO: return "GOTO";
        case TOKEN_SWITCH: return "SWITCH";
        case TOKEN_CASE: return "CASE";
        case TOKEN_DEFAULT: return "DEFAULT";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "LITERAL_INT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_STAR: return "STAR";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
        case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_GT: return "GT";
        case TOKEN_GE: return "GE";
        case TOKEN_LT: return "LT";
        case TOKEN_LE: return "LE";
        case TOKEN_BANG: return "BANG";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_INCREMENT: return "INCREMENT";
        case TOKEN_DECREMENT: return "DECREMENT";
        case TOKEN_PLUS_EQUAL: return "PLUSEQUAL";
        case TOKEN_MINUS_EQUAL: return "MINUSEQUAL";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_PERCENT: return "MOD";
        case TOKEN_AMPERSAND: return "AMPERSAND";
        case TOKEN_ASSERT_EXTENSION: return "_ASSERT";
        case TOKEN_PRINT_EXTENSION: return "_PRINT";
    }

    return "UNKNOWN";
}
