#ifndef _PARSER_UTIL_H_
#define _PARSER_UTIL_H_
#include "ast.h"

struct ASTNode * create_unary_node(ASTNodeType op, struct ASTNode * operand);
ASTNode * create_if_else_statement_node(ASTNode * condExpression, ASTNode * thenStatement, ASTNode * elseStatement);
ASTNode * create_while_statement_node(ASTNode * condExpression, ASTNode * bodyStatement);
ASTNode * create_ast_labeled_statement_node(const char * label, ASTNode * stmt);
ASTNode * create_ast_case_statement_node(ASTNode * constantExpression, ASTNode * stmt);
ASTNode * create_ast_default_statement_node(ASTNode * stmt);

ASTNodeType binary_op_token_to_ast_type(TokenType tok);
bool is_next_token_assignment(ParserContext * parserContext);
bool is_lvalue(ASTNode * node);

#endif