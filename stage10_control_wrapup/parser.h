#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "parser_context.h"

// extern char currentTokenInfo[128];
// extern char nextTokenInfo[128];

//void initialize_parser(ParserContext * parserContext, TokenList * tokenList);
//ASTNode* parse(ParserContext * parserContext);

ASTNode * parse(TokenList * tokenList);

#endif