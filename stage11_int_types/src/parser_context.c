#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "list_util.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "parser_util.h"
#include "parser_context.h"

char currentTokenInfo[128];
char nextTokenInfo[128];

void update_current_token_info(ParserContext* ctx) {
//    Token * currentToken = (ctx->pos < ctx->list->count) ? &ctx->list->data[ctx->pos] : NULL;
//    Token * currentToken = tokenlist_cursor_next(ctx->cursor);
    Token * currentToken = ctx->cursor->current->value;
    if (currentToken != NULL) {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos,
            token_type_name(currentToken->type), currentToken->text ? currentToken->text : "(null)");
    }
    else {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "(NULL)\n");
    }

//    Token * nextToken = (ctx->pos+1 < ctx->list->count) ? &ctx->list->data[ctx->pos+1] : NULL;
//    TokenData * curr = ctx->curr;
//    TokenData * next = curr->next;
    Token * nextToken = tokenlist_cursor_peek_next(ctx->cursor);
    if (nextToken != NULL) {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos+1,
            token_type_name(nextToken->type), nextToken->text ? nextToken->text : "(null)");
    }
    else {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "(NULL)\n");
    }
}

ParserContext* create_parser_context(tokenlist* tokens) {
    ParserContext * parserContext = malloc(sizeof(ParserContext));
    parserContext->cursor = malloc(sizeof(tokenlist_cursor));
    tokenlist_cursor_init(parserContext->cursor, tokens);
    parserContext->pos = 0;
    update_current_token_info(parserContext);
    return parserContext;
}

Token * peek(ParserContext * parserContext) {
//    return &parserContext->list->data[parserContext->pos];
    return parserContext->cursor->current->value;
}

bool is_current_token(ParserContext * parserContext, TokenType type) {
//    return parserContext->list->data[parserContext->pos].type == type;
    return parserContext->cursor->current->value->type == type;
}

//TODO need a check so this doesn't cause an out of bounds
bool is_next_token(ParserContext * parserContext, TokenType type) {
//    return parserContext->list->data[parserContext->pos+1].type == type;
    Token * nextToken = tokenlist_cursor_peek_next(parserContext->cursor);
    return nextToken->type == type;
}

Token * advance_parser(ParserContext * parserContext) {
    Token * token = peek(parserContext);

    tokenlist_cursor_next(parserContext->cursor);
    parserContext->pos++;
    update_current_token_info(parserContext);
    return token;
}

bool match_token(ParserContext * parserContext, TokenType type) {
//    if (parserContext->list->data[parserContext->pos].type == type) {
    if (parserContext->cursor->current->value->type == type) {
        advance_parser(parserContext);
        return true;
    }
    return false;
}

bool is_current_token_a_type(ParserContext * parserContext) {

    return is_current_token(parserContext, TOKEN_INT) ||
        is_current_token(parserContext, TOKEN_CHAR) ||
        is_current_token(parserContext, TOKEN_SHORT) ||
        is_current_token(parserContext, TOKEN_LONG);
}

Token* expect_token(ParserContext * parserContext, TokenType expected) {
    Token * token = peek(parserContext);
    if (token->type == expected) {
        return advance_parser(parserContext);
    }

    error("unexpected token at POS: %d, expected: %s, actual: %s\n", parserContext->pos, token_type_name(expected), token_type_name(token->type));
    return NULL;
}

Token * expect_type_token(ParserContext * ctx) {
    if (is_current_token_a_type(ctx)) {
        return advance_parser(ctx);
    }

    error("unexpected type token at POS: %d, expected: %s, actual: %s\n", ctx->pos, "int|char|short|long", token_type_name(peek(ctx)->type));
    return NULL;
}

const char * get_current_token_type(ParserContext * parserContext) {
    return token_type_name(peek(parserContext)->type);
}

int get_current_token_line(ParserContext * parserContext) {
    return parserContext->cursor->current->value->line;
}

int get_current_token_col(ParserContext * parserContext) {
    return parserContext->cursor->current->value->col;
}