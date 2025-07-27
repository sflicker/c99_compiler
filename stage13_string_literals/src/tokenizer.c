#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "list_util.h"
#include "token.h"
#include "tokenizer.h"
#include "tokenizer_context.h"
#include "util.h"
#include "error.h"

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
    { "[", TOKEN_LBRACKET },
    { "]", TOKEN_RBRACKET },
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
    { "&", TOKEN_AMPERSAND },
    { NULL, 0 }
};

Token * match_keyword(TokenizerContext * ctx, const char * text) {

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

void add_eof_token(tokenlist * tokens, int line, int col) {
    Token * eofToken = make_eof_token(line, col);
    add_token(tokens, eofToken);
}

void tokenize_number(TokenizerContext * ctx, tokenlist * tokens) {
    char buffer[128];
    int i=0;
    int line = ctx->line;
    int col = ctx->col;
    while(isdigit(ctx->curr_char)) {
        buffer[i++] = ctx->curr_char;
        advance(ctx);
    }
    buffer[i++] = '\0';
    add_token(tokens, make_int_token(buffer, line, col));

}

void add_identifier_token(tokenlist * tokens, const char * id, int line, int col) {
    
    Token * token = make_identifier_token(id, line, col);

    add_token(tokens, token);
}

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

tokenlist * tokenize(const char * text) {

    tokenlist * tokens = malloc(sizeof(tokenlist));
    tokenlist_init(tokens, free_token);

    TokenizerContext * ctx = init_tokenizer_context(text);
    
    Token * matched_tok = NULL;

    while(ctx->curr_char) {
        swallow_comment(ctx);
        if (isspace(ctx->curr_char)) {
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

            if ((matched_tok = match_keyword(ctx, buffer)) != NULL) {
                matched_tok->line = line;
                matched_tok->col = col;
                add_token(tokens, matched_tok);
            } else {
                add_identifier_token(tokens, buffer, line, col);
            }

        }
        else if (isdigit(ctx->curr_char)) {
            tokenize_number(ctx, tokens);
        }
        else if ((matched_tok = match_two_char_operator(ctx, ctx->curr_char, ctx->next_char)) != NULL) {
            add_token(tokens, matched_tok);
        }
        else if ((matched_tok = match_one_char_operator(ctx, ctx->curr_char)) != NULL) {
            add_token(tokens, matched_tok);
        }
        else {
            error("Invalid character '%c' at line: %d, col: %d\n", ctx->curr_char, ctx->line, ctx->col);
        }
    }

    add_eof_token(tokens, ctx->line, ctx->col);
    free_tokenizer_context(ctx);
    return tokens;
}

