#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "token.h"
#include "tokenizer.h"


const char *keywords[] = {
    "int", "return", "if", "else", "while", "for"
};

const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);

void init_token_list(TokenList * list);

void init_token_list(TokenList * list) {
    list->capacity = 16;
    list->count = 0;
    list->data = malloc(sizeof(Token) * list->capacity);
}

void add_token(TokenList * list, Token token) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, sizeof(Token) * list->capacity);
    }
    list->data[list->count++] = token;
}

bool is_keyword(const char * word) {
    for (int i=0;i<num_keywords; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_punctuator(char c) {
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

bool is_operator(char c) {
    return c == '*' || c == '+' || c == '-' || c == '/' || c == '!' || c == '=' ;
}

TokenType get_keyword_token(const char* keyword) {
    if (strcmp(keyword, "int") == 0) {
        return TOKEN_INT;
    }
    else if (strcmp(keyword, "return") == 0) {
        return TOKEN_RETURN;
    }
    else if (strcmp(keyword, "if") == 0) {
        return TOKEN_IF;
    }
    else if (strcmp(keyword, "else") == 0) {
        return TOKEN_ELSE;
    }
    else if (strcmp(keyword, "while") == 0) {
        return TOKEN_WHILE;
    }
    else if (strcmp(keyword, "for") == 0) {
        return TOKEN_FOR;
    }
    else {
        return TOKEN_UNKNOWN;
    }
}

TokenType punctuator_token(char punctuator) {
    switch (punctuator) {
        case '{': return TOKEN_LBRACE;
        case '}': return TOKEN_RBRACE;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case ';': return TOKEN_SEMICOLON;
        // case '+': return TOKEN_PLUS;
        // case '-': return TOKEN_MINUS;
        // case '*': return TOKEN_STAR;
        // case '/': return TOKEN_DIV;
        default : return TOKEN_UNKNOWN;
    }
}

TokenType single_char_operator(char operator) {
    switch (operator) {
        case '*': return TOKEN_STAR;
        case '+': return TOKEN_PLUS;
        case '-': return TOKEN_MINUS;
        case '/': return TOKEN_DIV;
        case '!': return TOKEN_BANG;
        case '=': return TOKEN_ASSIGN;
        default : return TOKEN_UNKNOWN;
    }
}


TokenType operator_token(const char * operator) {
    TokenType token = single_char_operator(operator[0]);
    return token;
}

const char * token_type_name(TokenType type) {
    switch(type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INT: return "INT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "LITERAL_INT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_STAR: return "STAR";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
        case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
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
        default: return "UNKNOWN";
    }
}

void add_token_by_type(TokenList * list, TokenType tokenType) {
    const char * tokenText = token_type_name(tokenType);
    int tokenLen = strlen(tokenText);
    char * tokenCopy = malloc(tokenLen + 1);
    memcpy(tokenCopy, tokenText, tokenLen);
    tokenCopy[tokenLen] = '\0';

    Token token;
    token.type = tokenType;
    token.text = tokenCopy;
    token.int_value = 0;
    token.length = tokenLen;
    add_token(list, token);
}


void add_int_token(TokenList * tokenList, char * numberText) {
    Token token;
    token.type = TOKEN_INT_LITERAL;
    int numberTextLen = strlen(numberText);
    char * numberTextCopy = malloc(numberTextLen + 1);
    memcpy(numberTextCopy, numberText, numberTextLen);
    numberTextCopy[numberTextLen] = '\0';
    token.text = numberTextCopy;
    token.length = numberTextLen;
    token.int_value = atoi(numberText);
    add_token(tokenList, token);
}


void add_keyword_token(TokenList *tokenList, const char * keywordText) {
    Token token;
    token.type = get_keyword_token(keywordText);
    int buffer_len = strlen(keywordText);
    char * buffer_copy = malloc(buffer_len + 1);
    memcpy(buffer_copy, keywordText, buffer_len);
    buffer_copy[buffer_len] = '\0';
    token.text = buffer_copy;
    token.length = buffer_len;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_identifier_token(TokenList * tokenList, const char * id) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    int idLen = strlen(id);
    char * idCopy = malloc(idLen + 1);
    memcpy(idCopy, id, idLen);
    idCopy[idLen] = '\0';
    token.text = idCopy;
    token.length = idLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_punctuator_token(TokenList * tokenList, const char * punctuatorText) {
    Token token;
    token.type = punctuator_token(*punctuatorText);
    int punctuatorLen = 1;
    char * punctuatorCopy = malloc(punctuatorLen + 1);
    memcpy(punctuatorCopy, punctuatorText, punctuatorLen);
    punctuatorCopy[punctuatorLen] = '\0';
    token.text = punctuatorCopy;
    token.length = punctuatorLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_operator_token(TokenList * tokenList, const char * operatorText) {
    Token token;
    token.type = operator_token(operatorText);
    int operatorLen = strlen(operatorText);
    char * operatorCopy = malloc(operatorLen + 1);
    memcpy(operatorCopy, operatorText, operatorLen);
    operatorCopy[operatorLen] = '\0';
    token.text = operatorCopy;
    token.length = operatorLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void tokenize(const char* code, TokenList * tokenList) {
    const char* p = code;
    
    init_token_list(tokenList);

    while(*p) {
        if (isspace(*p)) {
            p++;
        }
        else if (isalpha(*p) || *p == '_') {
            char buffer[128];
            int i=0;
            while(isalnum(*p) || *p == '_') {
                buffer[i++] = *p++;
            }
            buffer[i] = '\0';

            if (is_keyword(buffer)) {
                add_keyword_token(tokenList, buffer);
            }
            else {
                add_identifier_token(tokenList, buffer);
            }
        }
        else if (isdigit(*p)) {
            char buffer[128];
            int i=0;
            while(isdigit(*p)) {
                buffer[i++] = *p++;
            }
            buffer[i++] = '\0';
            add_int_token(tokenList, buffer);
        }
        else if (*p == '=' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_EQ);
            p += 2;
        }
        else if (*p == '!' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_NEQ);
            p += 2;
        }
        else if (*p == '>') {
            if (*(p+1) == '=') {
                add_token_by_type(tokenList, TOKEN_GE);
                p += 2;
            }
            else {
                add_token_by_type(tokenList, TOKEN_GT);
                p++;
            }
        }
        else if (*p == '<') {
            if (*(p+1) == '=') {
                add_token_by_type(tokenList, TOKEN_LE);
                p += 2;
            }
            else {
                add_token_by_type(tokenList, TOKEN_LT);
                p++;
            }
        }
        else if (*p == '+' && *(p+1) == '+') {
            add_token_by_type(tokenList, TOKEN_INCREMENT);
            p+=2;
        }
        else if (*p == '-' && *(p+1) == '-') {
            add_token_by_type(tokenList, TOKEN_DECREMENT);
            p+=2;
        }
        else if (*p == '+' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_PLUS_EQUAL);
            p+=2;
        }
        else if (*p == '-' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_MINUS_EQUAL);
            p+=2;
        }
        else if (*p == '&' && *(p+1) == '&') {
            add_token_by_type(tokenList, TOKEN_LOGICAL_AND);
            p+=2;
        }
        else if (*p == '|' && *(p+1) == '|') {
            add_token_by_type(tokenList, TOKEN_LOGICAL_OR);
            p+=2;
        }
        else if (is_punctuator(*p)) {
            char * buffer = malloc(2);
            memcpy(buffer, p, 1);
            buffer[1] = '\0';
            add_punctuator_token(tokenList, buffer);
            p++;
        }
        else if (is_operator(*p)) {
            char * buffer = malloc(2);
            memcpy(buffer, p, 1);
            buffer[1] = '\0';
            add_operator_token(tokenList, buffer);
            p++;
        }

    }

    Token eofToken;
    eofToken.type = TOKEN_EOF;
    eofToken.text = '\0';
    eofToken.length = 0;
    add_token(tokenList, eofToken);

}

void cleanup_token_list(TokenList * tokenList) {
    for (int i=0;i<tokenList->count;i++) {
        free((void*)tokenList->data[i].text);
    }
}