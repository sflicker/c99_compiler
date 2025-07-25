#ifndef __PARSER_CONTEXT_H__
#define __PARSER_CONTEXT_H__

#include <stdbool.h>

#include "list_util.h"
#include "token.h"

typedef struct {
    tokenlist_cursor * cursor;
    int pos;
    const char * current_decl_name;
    ASTNode_list * astParam_list;
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

void set_current_decl_name(ParserContext * parserContext, const char * name);
const char * get_current_decl_name(ParserContext * parserContext);

void set_current_decl_param_list(ParserContext * parserContext, ASTNode_list * astParam_list);
ASTNode_list * get_current_decl_param_list(ParserContext * parserContext);

#endif