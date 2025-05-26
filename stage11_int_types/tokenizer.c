#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "token.h"
#include "tokenizer.h"
#include "util.h"

//bool is_keyword(const char * word);

// const char *keywords[] = {
//     "int", "return", "if", "else", "while", "for", "break", "continue", "_assert", "_print"
// };

typedef struct {
    const char* text;
    TokenType type;
} TokenMapEntry;

TokenMapEntry keyword_map[] = {
    { "int", TOKEN_INT }, 
    { "char", TOKEN_CHAR },
    { "short", TOKEN_SHORT },
    { "long", TOKEN_LONG },
    { "return", TOKEN_RETURN },
    { "if", TOKEN_IF },
    { "else", TOKEN_ELSE },
    { "while", TOKEN_WHILE },
    { "for", TOKEN_FOR },
    { "break", TOKEN_BREAK },
    { "continue", TOKEN_CONTINUE },
    { "do", TOKEN_DO },
    { "goto", TOKEN_GOTO },
    { "switch", TOKEN_SWITCH },
    { "case", TOKEN_CASE },
    { "default", TOKEN_DEFAULT },
    { "_assert", TOKEN_ASSERT_EXTENSION },
    { "_print", TOKEN_PRINT_EXTENSION },
    { NULL, 0 }
};

TokenMapEntry two_char_operator_map[] = {
    { "==", TOKEN_EQ },
    { "!=", TOKEN_NEQ },
    { "<=", TOKEN_LE },
    { ">=", TOKEN_GE },
    { "&&", TOKEN_LOGICAL_AND },
    { "||", TOKEN_LOGICAL_OR },
    { "+=", TOKEN_PLUS_EQUAL },
    { "-=", TOKEN_MINUS_EQUAL },
    { "++", TOKEN_INCREMENT },
    { "--", TOKEN_DECREMENT },
    { NULL, 0 }
};

TokenMapEntry single_char_operator_map[] = {
    { "(", TOKEN_LPAREN },
    { ")", TOKEN_RPAREN },
    { "{", TOKEN_LBRACE },
    { "}", TOKEN_RBRACE },
    { ";", TOKEN_SEMICOLON },
    { "-", TOKEN_MINUS },
    { "+", TOKEN_PLUS },
    { "*", TOKEN_STAR },
    { "/", TOKEN_DIV },
    { "=", TOKEN_ASSIGN },
    { ",", TOKEN_COMMA },
    { "!", TOKEN_BANG },
    { "<", TOKEN_LT },
    { ">", TOKEN_GT },
    { "%", TOKEN_PERCENT },
    { ":", TOKEN_COLON },
    { NULL, 0 }
};

//const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);

Token * make_token(TokenType type, const char * text, int line, int col) {
    Token * token = malloc(sizeof(Token));
    token->type = type;
    token->text = text;
    token->line = line;
    token->col = col;
    return token;
}

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

// bool is_keyword(const char * word) {
//     for (int i=0;i<num_keywords; i++) {
//         if (strcmp(word, keywords[i]) == 0) {
//             return true;
//         }
//     }
//     return false;
// }

// bool is_punctuator(char c) {
//     return c == '{' || c == '}' || c == '(' || c == ')' || c == ';' || c == ',';
// }

// bool is_operator(char c) {
//     return c == '*' || c == '+' || c == '-' || c == '/' || c == '!' || c == '=' ;
// }

// TokenType get_keyword_token(const char* keyword) {
//     if (strcmp(keyword, "int") == 0) {
//         return TOKEN_INT;
//     }
//     else if (strcmp(keyword, "return") == 0) {
//         return TOKEN_RETURN;
//     }
//     else if (strcmp(keyword, "if") == 0) {
//         return TOKEN_IF;
//     }
//     else if (strcmp(keyword, "else") == 0) {
//         return TOKEN_ELSE;
//     }
//     else if (strcmp(keyword, "while") == 0) {
//         return TOKEN_WHILE;
//     }
//     else if (strcmp(keyword, "for") == 0) {
//         return TOKEN_FOR;
//     }
//     else if (strcmp(keyword, "_assert") == 0) {
//         return TOKEN_ASSERT_EXTENSION;
//     }
//     else if (strcmp(keyword, "_print") == 0) {
//         return TOKEN_PRINT_EXTENSION;
//     }
//     else {
//         return TOKEN_UNKNOWN;
//     }
// }

Token * match_keyword(TokenizerContext * ctx, char * text) {

    for (int i=0;keyword_map[i].text != NULL; i++) {
        if (strcmp(text, keyword_map[i].text) == 0) {
            return make_token(keyword_map[i].type, keyword_map[i].text, ctx->line, ctx->col);
        }
    }
    return NULL;

}

Token * match_two_char_operator(TokenizerContext *ctx, char first, char second) {
    char op_str[3] = { first, second, '\0' };
    for (int i=0;two_char_operator_map[i].text != NULL; i++) {
        if (strcmp(op_str, two_char_operator_map[i].text) == 0) {
            Token *tok = make_token(two_char_operator_map[i].type, two_char_operator_map[i].text, ctx->line, ctx->col);
            advance(ctx);
            advance(ctx);
            return tok;
        }
    }
    return NULL;
}

Token * match_one_char_operator(TokenizerContext * ctx, char c) {
    char match_str[2] = { c, '\0' };
    for (int i=0;single_char_operator_map[i].text != NULL; i++) {
        if (strcmp(match_str, single_char_operator_map[i].text) == 0) {
            Token *tok = make_token(single_char_operator_map[i].type, single_char_operator_map[i].text, ctx->line, ctx->col);
            advance(ctx);
            return tok;
        }
    }
    return NULL;
}

TokenType punctuator_token(char punctuator) {
    switch (punctuator) {
        case '{': return TOKEN_LBRACE;
        case '}': return TOKEN_RBRACE;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case ';': return TOKEN_SEMICOLON;
        case ',': return TOKEN_COMMA;
        
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
        case TOKEN_ASSERT_EXTENSION: return "_ASSERT";
        case TOKEN_PRINT_EXTENSION: return "_PRINT";

        default: return "UNKNOWN";
    }
}

// void add_token_by_type(TokenList * list, TokenType tokenType) {
//     const char * tokenText = token_type_name(tokenType);
//     int tokenLen = strlen(tokenText);
//     char * tokenCopy = malloc(tokenLen + 1);
//     memcpy(tokenCopy, tokenText, tokenLen);
//     tokenCopy[tokenLen] = '\0';

//     Token token;
//     token.type = tokenType;
//     token.text = tokenCopy;
//     token.int_value = 0;
//     token.length = tokenLen;
//     add_token(list, token);
// }


void add_int_token(TokenList * tokenList, char * numberText, int line, int col) {
    Token token;
    token.type = TOKEN_INT_LITERAL;
    int numberTextLen = strlen(numberText);
    char * numberTextCopy = malloc(numberTextLen + 1);
    memcpy(numberTextCopy, numberText, numberTextLen);
    numberTextCopy[numberTextLen] = '\0';
    token.text = numberTextCopy;
    token.length = numberTextLen;
    token.int_value = atoi(numberText);
    token.line = line;
    token.col = col;
    add_token(tokenList, token);
}

void tokenize_number(TokenizerContext * ctx, TokenList * tokenList) {
    char buffer[128];
    int i=0;
    int line = ctx->line;
    int col = ctx->col;
    while(isdigit(ctx->curr_char)) {
        buffer[i++] = ctx->curr_char;
        advance(ctx);
    }
    buffer[i++] = '\0';
    add_int_token(tokenList, buffer, line, col);

}

void add_identifier_token(TokenList * tokenList, const char * id, int line, int col) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    int idLen = strlen(id);
    char * idCopy = malloc(idLen + 1);
    memcpy(idCopy, id, idLen);
    idCopy[idLen] = '\0';
    token.text = idCopy;
    token.length = idLen;
    token.int_value = 0;
    token.line = line;
    token.col = col;
    add_token(tokenList, token);
}

// void add_punctuator_token(TokenList * tokenList, const char * punctuatorText) {
//     Token token;
//     token.type = punctuator_token(*punctuatorText);
//     int punctuatorLen = 1;
//     char * punctuatorCopy = malloc(punctuatorLen + 1);
//     memcpy(punctuatorCopy, punctuatorText, punctuatorLen);
//     punctuatorCopy[punctuatorLen] = '\0';
//     token.text = punctuatorCopy;
//     token.length = punctuatorLen;
//     token.int_value = 0;
//     add_token(tokenList, token);
// }

// void add_operator_token(TokenList * tokenList, const char * operatorText) {
//     Token token;
//     token.type = operator_token(operatorText);
//     int operatorLen = strlen(operatorText);
//     char * operatorCopy = malloc(operatorLen + 1);
//     memcpy(operatorCopy, operatorText, operatorLen);
//     operatorCopy[operatorLen] = '\0';
//     token.text = operatorCopy;
//     token.length = operatorLen;
//     token.int_value = 0;
//     add_token(tokenList, token);
// }

void swallow_comment(TokenizerContext * ctx) {
    if (ctx->curr_char == '/') {
        if (ctx->next_char == '/') {
            advance(ctx);
            advance(ctx);
            while (ctx->curr_char != '\n' && ctx->curr_char != '\0') {
                advance(ctx);    // skip to end of line
            }
        }
        else if (ctx->next_char == '*') {
            advance(ctx);
            advance(ctx);
            while (!(ctx->curr_char == '*' && ctx->next_char == '/')) {
                if (ctx->curr_char == '\0') {
                    error("Unterminated block comment\n");
                    exit(1);
                }
                advance(ctx);
            }
            advance(ctx);
            advance(ctx);
        }
    }
}


void tokenize(TokenizerContext * ctx, TokenList * tokenList) {
    
    init_token_list(tokenList);
    Token * matched_tok = NULL;

    while(ctx->curr_char) {
        swallow_comment(ctx);
        if (isspace(ctx->curr_char)) {
            //p++;
            advance(ctx);
        }
        else if (isalpha(ctx->curr_char) || ctx->curr_char == '_') {
            char buffer[128];
            int i=0;
            int line = ctx->line;
            int col = ctx->col;
            while(isalnum(ctx->curr_char) || ctx->curr_char == '_') {
                  buffer[i++] = ctx->curr_char;
                  advance(ctx);
            }
            buffer[i] = '\0';

            // if (is_keyword(buffer)) {
            //     add_keyword_token(tokenList, buffer);
            // }
            // else {
            //     add_identifier_token(tokenList, buffer);
            // }

            if ((matched_tok = match_keyword(ctx, buffer)) != NULL) {
                add_token(tokenList, *matched_tok);
            } else {
                add_identifier_token(tokenList, buffer, line, col);
            }

        }
        else if (isdigit(ctx->curr_char)) {
            tokenize_number(ctx, tokenList);
            // char buffer[128];
            // int i=0;
            // while(isdigit(ctx->curr_char)) {
            //     buffer[i++] = ctx->curr_char;
            //     advance(ctx);
            // }
            // buffer[i++] = '\0';
            // add_int_token(tokenList, buffer);
        }
        else if ((matched_tok = match_two_char_operator(ctx, ctx->curr_char, ctx->next_char)) != NULL) {
            add_token(tokenList, *matched_tok);
        }
        else if ((matched_tok = match_one_char_operator(ctx, ctx->curr_char)) != NULL) {
            add_token(tokenList, *matched_tok);
        }
        // else if (ctx->curr_char == '=' && ctx->next_char == '=') {
        //     add_token_by_type(tokenList, TOKEN_EQ);
        //     //p += 2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '!' && ctx->next_char == '=') {
        //     add_token_by_type(tokenList, TOKEN_NEQ);
        //     //p += 2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '>') {
        //     if (ctx->next_char == '=') {
        //         add_token_by_type(tokenList, TOKEN_GE);
        //         //p += 2;
        //         advance(ctx);
        //         advance(ctx);
        //     }
        //     else {
        //         add_token_by_type(tokenList, TOKEN_GT);
        //         //p++;
        //         advance(ctx);
        //     }
        // }
        // else if (ctx->curr_char == '<') {
        //     if (ctx->next_char == '=') {
        //         add_token_by_type(tokenList, TOKEN_LE);
        //         //p += 2;
        //         advance(ctx);
        //         advance(ctx);
        //     }
        //     else {
        //         add_token_by_type(tokenList, TOKEN_LT);
        //         //p++;
        //         advance(ctx);
        //     }
        // }
        // else if (ctx->curr_char == '+' && ctx->next_char == '+') {
        //     add_token_by_type(tokenList, TOKEN_INCREMENT);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '-' && ctx->next_char == '-') {
        //     add_token_by_type(tokenList, TOKEN_DECREMENT);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '+' && ctx->next_char == '=') {
        //     add_token_by_type(tokenList, TOKEN_PLUS_EQUAL);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '-' && ctx->next_char == '=') {
        //     add_token_by_type(tokenList, TOKEN_MINUS_EQUAL);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '&' && ctx->next_char == '&') {
        //     add_token_by_type(tokenList, TOKEN_LOGICAL_AND);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (ctx->curr_char == '|' && ctx->next_char == '|') {
        //     add_token_by_type(tokenList, TOKEN_LOGICAL_OR);
        //     //p+=2;
        //     advance(ctx);
        //     advance(ctx);
        // }
        // else if (is_punctuator(ctx->curr_char)) {
        //     char * buffer = malloc(2);
        //     memcpy(buffer, &ctx->curr_char, 1);
        //     buffer[1] = '\0';
        //     add_punctuator_token(tokenList, buffer);
        //     //p++;
        //     advance(ctx);
        // }
        // else if (is_operator(ctx->curr_char)) {
        //     char * buffer = malloc(2);
        //     memcpy(buffer,&ctx->curr_char, 1);
        //     buffer[1] = '\0';
        //     add_operator_token(tokenList, buffer);
        //     //p++;
        //     advance(ctx);
        // }
        else {
            error("Invalid character '%c' at line: %d, col: %d\n", ctx->curr_char, ctx->line, ctx->col);

        }
    }

    Token eofToken;
    eofToken.type = TOKEN_EOF;
    eofToken.text = '\0';
    eofToken.length = 0;
    eofToken.line = ctx->line;
    eofToken.col = ctx->col;
    add_token(tokenList, eofToken);

}

void cleanup_token_list(TokenList * tokenList) {
    for (int i=0;i<tokenList->count;i++) {
        free((void*)tokenList->data[i].text);
    }
}