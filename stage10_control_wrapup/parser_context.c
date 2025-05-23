#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "parser_util.h"
#include "parser_context.h"

char currentTokenInfo[128];
char nextTokenInfo[128];

void update_current_token_info(ParserContext* ctx) {
    Token * currentToken = (ctx->pos < ctx->list->count) ? &ctx->list->data[ctx->pos] : NULL;
    if (currentToken != NULL) {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos,
            token_type_name(currentToken->type), currentToken->text ? currentToken->text : "(null)");
    }
    else {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "(NULL)\n");
    }

    Token * nextToken = (ctx->pos+1 < ctx->list->count) ? &ctx->list->data[ctx->pos+1] : NULL;
    if (nextToken != NULL) {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos+1,
            token_type_name(nextToken->type), nextToken->text ? nextToken->text : "(null)");
    }
    else {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "(NULL)\n");
    }
}

ParserContext* create_parser_context(TokenList * tokenList) {
    ParserContext * parserContext = malloc(sizeof(ParserContext));
    parserContext->list = tokenList;
    parserContext->pos = 0;
    update_current_token_info(parserContext);
    return parserContext;
}

Token * peek(ParserContext * parserContext) {
    return &parserContext->list->data[parserContext->pos];
}

bool is_current_token(ParserContext * parserContext, TokenType type) {
    return parserContext->list->data[parserContext->pos].type == type;
}

//TODO need a check so this doesn't cause an out of bounds
bool is_next_token(ParserContext * parserContext, TokenType type) {
    return parserContext->list->data[parserContext->pos+1].type == type;
}

Token * advance_parser(ParserContext * parserContext) {
    Token * token = peek(parserContext);
    parserContext->pos++;
    update_current_token_info(parserContext);
    return token;
}

bool match_token(ParserContext * parserContext, TokenType type) {
    if (parserContext->list->data[parserContext->pos].type == type) {
        advance_parser(parserContext);
        return true;
    }
    return false;
}

Token* expect_token(ParserContext * parserContext, TokenType expected) {
    Token * token = peek(parserContext);
    if (token->type == expected) {
        return advance_parser(parserContext);
    }

    printf("unexpected token at POS: %d, expected: %s, actual: %s\n", parserContext->pos, token_type_name(expected), token_type_name(token->type));

    exit(1);    
}