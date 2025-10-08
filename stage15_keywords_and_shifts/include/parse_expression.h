//
// Created by scott on 8/16/25.
//

#ifndef _PARSE_EXPRESSION_H
#define _PARSE_EXPRESSION_H

#include "parser_context.h"

ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_constant_expression(ParserContext * parserContext);
ASTNode * parse_assignment_expression(ParserContext * parserContext);
ASTNode * parse_conditional_expression(ParserContext * parserContext);
ASTNode * parse_logical_or(ParserContext * parserContext);
ASTNode * parse_logical_and(ParserContext * parserContext);
ASTNode * parse_equality_expression(ParserContext * parserContext);
ASTNode * parse_relational_expression(ParserContext * parserContext);
ASTNode * parse_additive_expression(ParserContext * parserContext);
ASTNode * parse_multiplicative_expression(ParserContext * parserContext);
ASTNode * parse_cast_expression(ParserContext * parserContext);
ASTNode * parse_unary_expression(ParserContext * parserContext);
ASTNode * parse_postfix_expression(ParserContext * parserContext);
ASTNode * parse_primary_expression(ParserContext * parserContext);

#endif //_PARSE_EXPRESSION_H