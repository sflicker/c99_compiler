#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
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

const char *keywords[] = {
    "int", "return"
};

const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);

bool is_keyword(const char * word) {
    for (int i=0;i<num_keywords; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}


const char * sampleProgram = "int main() { \n"
                             "    return 42; \n"
                             "}";


bool is_punctuator(char c) {
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

TokenType punctuator_token(char punctuator) {
    switch (punctuator) {
        case '{': return TOKEN_LBRACE;
        case '}': return TOKEN_RBRACE;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case ';': return TOKEN_SEMICOLON;
        default : return TOKEN_UNKNOWN;
    }
}

TokenType get_keyword_token(const char* keyword) {
    if (strcmp(keyword, "int") == 0) {
        return TOKEN_INT;
    }
    else if (strcmp(keyword, "return") == 0) {
        return TOKEN_RETURN;
    }
    else {
        return TOKEN_UNKNOWN;
    }
}

const char * token_type_name(TokenType type) {
    switch(type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INT: return "INT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "LITERAL_INT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        default: return "UNKNOWN";
    }
}

void formatted_output(const char * label, const char * text, TokenType tokenType) {
    char left[64];
    char right[64];
    snprintf(left, sizeof(left), "%s: %s", label, text);
    snprintf(right, sizeof(right), "TOKEN_TYPE: %s", token_type_name(tokenType));
    printf("%-25s %-25s\n", left, right);
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
    token.text = idCopy;
    token.length = idLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_int_token(TokenList * tokenList, char * numberText) {
    Token token;
    token.type = TOKEN_INT_LITERAL;
    int numberTextLen = strlen(numberText);
    char * numberTextCopy = malloc(numberTextLen + 1);
    memcpy(numberTextCopy, numberText, numberTextLen);
    token.text = numberTextCopy;
    token.length = numberTextLen;
    token.int_value = atoi(numberText);
    add_token(tokenList, token);
}


void add_punctuator_token(TokenList * tokenList, const char * punctuatorText) {
    Token token;
    token.type = punctuator_token(*punctuatorText);
    int punctuatorLen = 1;
    char * punctuatorCopy = malloc(punctuatorLen + 1);
    memcpy(punctuatorCopy, punctuatorText, punctuatorLen);
    token.text = punctuatorCopy;
    token.length = punctuatorLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void tokenize(const char* code) {
    const char* p = code;
    
    TokenList tokenList;
    init_token_list(&tokenList);

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
                add_keyword_token(&tokenList, buffer);

//                formatted_output("KEYWORD:", buffer, tokenType);
            }
            else {
                add_identifier_token(&tokenList, buffer);

 //               formatted_output("IDENTIFIER:", buffer, tokenType);
            }
        }
        else if (isdigit(*p)) {
            char buffer[128];
            int i=0;
            while(isdigit(*p)) {
                buffer[i++] = *p++;
            }
            buffer[i++] = '\0';
            add_int_token(&tokenList, buffer);
//            formatted_output("INTEGER:" , buffer, tokenType);
        }
        else if (is_punctuator(*p)) {
            char buffer[2] = { *p, '\0'};
            add_punctuator_token(&tokenList, buffer);

//            formatted_output("PUNCTUATOR:", buffer, tokenType);
            p++;
        }
    }

    // output list
    for (int i=0;i<tokenList.count;i++) {
        Token token = tokenList.data[i];
        formatted_output("TOKEN:", token.text, token.type);
    }

    // cleanup (do else where if list returned)
    for (int i=0;i<tokenList.count;i++) {
        free((void*)tokenList.data[i].text);
    }

}

int main() {
    tokenize(sampleProgram);
}                             