#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include "parser_context.h"

// extern char currentTokenInfo[128];
// extern char nextTokenInfo[128];

//void initialize_parser(ParserContext * parserContext, TokenList * tokenList);
//ASTNode* parse(ParserContext * parserContext);

ASTNode * parse(tokenlist * tokenList);
ASTNode * parse_translation_unit(ParserContext * parserContext);
ASTNode * parse_external_declaration(ParserContext * parserContext);
ASTNode * parse_function(ParserContext * parserContext);
ASTNode * parse_function_with_type(ParserContext * parserContext, CType * ctype);
ASTNode*  parse_global_var_decl(ParserContext * parserContext, CType * ctype);
ASTNode * parse_statement(ParserContext* parserContext);
ASTNode * parse_return_statement(ParserContext * parserContext);
ASTNode * parse_block(ParserContext* parserContext);
ASTNode * parse_if_statement(ParserContext * parserContext);
ASTNode * parse_expression_statement(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext* parserContext);
ASTNode * parse_equality_expression(ParserContext * parserContext);
ASTNode * parse_relational_expression(ParserContext * parserContext);
ASTNode * parse_additive_expression(ParserContext * parserContext);
ASTNode * parse_multiplicative_expression(ParserContext * parserContext);
ASTNode * parse_cast_expression(ParserContext * parserContext);
ASTNode * parse_unary_expression(ParserContext * parserContext);
ASTNode * parse_postfix_expression(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_constant_expression(ParserContext * parserContext);
ASTNode * parse_primary(ParserContext * parserContext);
ASTNode * parse_var_declaration(ParserContext * parserContext);
ASTNode * parse_assignment_statement(ParserContext * parserContext);
ASTNode * parse_while_statement(ParserContext * parserContext);
ASTNode * parse_for_statement(ParserContext * parserContext);
ASTNode * parse_assignment_expression(ParserContext * parserContext);
ASTNode * parse_logical_or(ParserContext * parserContext);
ASTNode * parse_logical_and(ParserContext * parserContext);
ASTNode * parse_assert_extension_statement(ParserContext * parserContext);
ASTNode * parse_print_extension_statement(ParserContext * parserContext);
CType * parse_ctype(ParserContext * ctx);

#endif