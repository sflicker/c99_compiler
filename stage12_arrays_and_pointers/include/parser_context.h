#ifndef __PARSER_CONTEXT_H__
#define __PARSER_CONTEXT_H__

#include <stdbool.h>

#include "list_util.h"
#include "token.h"

typedef struct {
    tokenlist_cursor * cursor;
    int pos;
} ParserContext;


ParserContext* create_parser_context(tokenlist * tokenList);
void free_parser_context(ParserContext* parserContext);

Token * peek(ParserContext * parserContext);
Token * peek_next(ParserContext * parserContext);
bool is_current_token(ParserContext * parserContext, TokenType type);
bool is_next_token(ParserContext * parserContext, TokenType type);
Token * advance_parser(ParserContext * parserContext);
bool match_token(ParserContext * parserContext, TokenType type);
Token* expect_token(ParserContext * parserContext, TokenType expected);
const char * get_current_token_type_name(ParserContext * parserContext);
int get_current_token_line(ParserContext * parserContext);
int get_current_token_col(ParserContext * parserContext);
bool is_current_token_a_ctype(ParserContext * parserContext);
bool is_next_token_a_ctype(ParserContext * parserContext);
Token * expect_ctype_token(ParserContext * ctx);

#endif