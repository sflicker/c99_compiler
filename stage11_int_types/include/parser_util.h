#ifndef _PARSER_UTIL_H_
#define _PARSER_UTIL_H_
#include "ast.h"

ASTNode * create_translation_unit_node(ASTNode_list * functions);
//ASTNode * create_unary_node(ASTNodeType op, struct ASTNode * operand);
ASTNode * create_unary_node(UnaryOperator op, ASTNode * operand);
ASTNode * create_binary_op_node(ASTNode * lhs, BinaryOperator  op, ASTNode *rhs);
ASTNode * create_if_else_statement_node(ASTNode * condExpression, ASTNode * thenStatement, ASTNode * elseStatement);
ASTNode * create_while_statement_node(ASTNode * condExpression, ASTNode * bodyStatement);
ASTNode * create_ast_labeled_statement_node(const char * label, ASTNode * stmt);
ASTNode * create_ast_case_statement_node(ASTNode * constantExpression, ASTNode * stmt);
ASTNode * create_ast_default_statement_node(ASTNode * stmt);
ASTNode * create_goto_statement(const char * label);
ASTNode * create_do_while_statement(ASTNode * stmt, ASTNode * expr);
ASTNode * create_switch_statement(ASTNode * expr, ASTNode * stmt);
ASTNode * create_break_statement_node();
ASTNode * create_continue_statement_node();
ASTNode * create_int_literal_node(int value);
ASTNode * create_function_call_node(const char * name, ASTNode_list * args);
ASTNode * create_var_ref_node(const char * name);
ASTNode * create_var_decl_node(const char * name, CType * ctype, ASTNode * init_expr);
ASTNode * create_for_statement_node(ASTNode * init_expr, ASTNode * cond_expr,
            ASTNode * update_expr, ASTNode * body);
            
BinaryOperator binary_op_token_to_ast_binop_type(TokenType tok);
bool is_next_token_assignment(ParserContext * parserContext);
bool is_lvalue(ASTNode * node);

#endif