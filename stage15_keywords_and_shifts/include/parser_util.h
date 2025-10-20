#ifndef _PARSER_UTIL_H_
#define _PARSER_UTIL_H_
#include "ast.h"
#include "parser.h"
#include "parser_context.h"
#include "parse_declaration.h"

ASTNode * create_translation_unit_node(ASTNode_list * functions, ASTNode_list * globals,
    ASTNode_list * string_literals, ASTNode_list * float_literals, ASTNode_list * double_literals) ;
ASTNode * create_unary_node(UnaryOperator op, ASTNode * operand);
ASTNode * create_binary_node(ASTNode * lhs, BinaryOperator  op, ASTNode *rhs);
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
ASTNode * create_float_literal_node(float value);
ASTNode * create_double_literal_node(double value);
ASTNode * create_function_call_node(const char * name, ASTNode_list * args);
ASTNode * create_var_ref_node(const char * name);
ASTNode * create_var_decl_node(const char * name, CType * ctype, ASTNode * init_expr);
ASTNode * create_for_statement_node(ASTNode * init_expr, ASTNode * cond_expr,
            ASTNode * update_expr, ASTNode * body);
// ASTNode * create_function_declaration_node(const char * name, CType * returnType,
//         ASTNode_list * param_list, ASTNode * body, bool declaration_only);
ASTNode * create_function_declaration_node(const char * name, CType * func_type,
        ASTNode_list * param_list);
ASTNode * create_function_definition_node(const char * name, CType * func_type,
        ASTNode_list * param_list, ASTNode * body);
ASTNode * create_return_statement_node(ASTNode * expr);
ASTNode * create_expression_statement_node(ASTNode * expr);
ASTNode * create_block_node(ASTNode_list * stmts);
ASTNode * create_cast_expr_node(CType * target_type, ASTNode * expr);
ASTNode * create_array_access_node(ASTNode * arrayNode, ASTNode * index);
ASTNode * create_initializer_list(ASTNode_list * list);
ASTNode * create_print_extension_node(ASTNode * expr);
ASTNode * create_assert_extension_node(ASTNode * expr);
ASTNode * create_string_literal_node(const char *text);
ASTNode * create_declaration_node(CType * ctype);
ASTNode * create_cond_expr_node(ASTNode * condExpression, ASTNode * thenExpression, ASTNode * elseExpression);
ASTNode * create_sizeof_node_with_type(CType * ctype);
ASTNode * create_sizeof_node_with_expression(ASTNode * operand);
BinaryOperator binary_op_token_to_ast_binop_type(TokenType tok);
bool is_next_token_assignment(ParserContext * parserContext);
CType * get_ctype_from_token(Token* token);

Declarator * make_declarator();

#endif