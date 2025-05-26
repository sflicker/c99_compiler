#ifndef __PARSER_CONTEXT_H__
#define __PARSER_CONTEXT_H__

#include "token.h"

typedef struct {
    TokenList* list;
    int pos;
} ParserContext;


ParserContext* create_parser_context(TokenList * tokenList);
Token * peek(ParserContext * parserContext);
bool is_current_token(ParserContext * parserContext, TokenType type);
bool is_next_token(ParserContext * parserContext, TokenType type);
Token * advance_parser(ParserContext * parserContext);
bool match_token(ParserContext * parserContext, TokenType type);
Token* expect_token(ParserContext * parserContext, TokenType expected);
const char * get_current_token_type(ParserContext * parserContext);
int get_current_token_line(ParserContext * parserContext);
int get_current_token_col(ParserContext * parserContext);
bool is_current_token_a_type(ParserContext * parserContext);
Token * expect_type_token(ParserContext * ctx);

#endif