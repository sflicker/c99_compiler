#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

typedef struct {
    TokenList* list;
    int pos;
} ParserContext;

ASTNode* parse_program(ParserContext * parserContext);

#endif