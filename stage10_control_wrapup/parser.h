#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

extern char currentTokenInfo[128];
extern char nextTokenInfo[128];

typedef struct {
    TokenList* list;
    int pos;
} ParserContext;

void initialize_parser(ParserContext * parserContext, TokenList * tokenList);
ASTNode* parse(ParserContext * parserContext);
void print_ast(ASTNode * node, int indent);

#endif