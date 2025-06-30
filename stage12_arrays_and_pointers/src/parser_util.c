#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <error.h>
#include "util.h"
#include "list_util.h"
#include "ast_list.h"
#include "token.h"
#include "ast.h"
#include "c_type.h"
#include "parser_context.h"
#include "parser_util.h"

ASTNode * create_translation_unit_node(ASTNode_list * functions, ASTNode_list * globals) {
    ASTNode * node = create_ast();
    node->type = AST_TRANSLATION_UNIT;
    node->translation_unit.globals = globals;
    node->translation_unit.functions = functions;
    node->translation_unit.count = functions->count;
    node->symbol = NULL;
    return node;
}

ASTNode * create_unary_node(UnaryOperator op, ASTNode * operand) {
    ASTNode * node = create_ast();
    node->type = AST_UNARY_EXPR;
    node->unary.operand = operand;
    node->unary.op = op;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_binary_node(ASTNode * lhs, BinaryOperator op, ASTNode *rhs) {
    ASTNode * node = create_ast();
    node->type = AST_BINARY_EXPR;
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

BinaryOperator binary_op_token_to_ast_binop_type(TokenType tok) {
    switch (tok) {
        case TOKEN_PLUS: return BINOP_ADD;
        case TOKEN_MINUS: return BINOP_SUB;
        case TOKEN_STAR: return BINOP_MUL;
        case TOKEN_DIV: return BINOP_DIV;
        case TOKEN_PERCENT: return BINOP_MOD;
        case TOKEN_EQ: return BINOP_EQ;
        case TOKEN_NEQ: return BINOP_NE;
        case TOKEN_LT: return BINOP_LT;
        case TOKEN_LE: return BINOP_LE;
        case TOKEN_GT: return BINOP_GT;
        case TOKEN_GE: return BINOP_GE;
        default:
            fprintf(stderr, "Unknown binary operator in expression");
            exit(1);
    }
}

bool is_next_token_assignment(ParserContext * parserContext) {
    return is_next_token(parserContext, TOKEN_ASSIGN) || 
        is_next_token(parserContext, TOKEN_PLUS_EQUAL) || 
        is_next_token(parserContext, TOKEN_MINUS_EQUAL);
}

ASTNode * create_if_else_statement_node(ASTNode * condExpression, ASTNode * thenStatement, ASTNode * elseStatement) {
    ASTNode * node = create_ast();
    node->type = AST_IF_STMT;
    node->if_stmt.cond = condExpression;
    node->if_stmt.then_stmt = thenStatement;
    node->if_stmt.else_stmt = elseStatement;
    node->symbol = NULL;
    return node;
}

ASTNode * create_while_statement_node(ASTNode * condExpression, ASTNode * bodyStatement) {
    ASTNode * node = create_ast();
    node->type = AST_WHILE_STMT;
    node->while_stmt.cond = condExpression;
    node->while_stmt.body = bodyStatement;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_ast_labeled_statement_node(const char * label, ASTNode * stmt) {
    ASTNode * node = create_ast();
    node->type = AST_LABELED_STMT;
    node->labeled_stmt.label = strdup(label);
    node->labeled_stmt.stmt = stmt;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_ast_case_statement_node(ASTNode * constantExpression, ASTNode * stmt) {
    ASTNode * node = create_ast();
    node->type = AST_CASE_STMT;
    node->case_stmt.constExpression = constantExpression;
    node->case_stmt.stmt = stmt;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_ast_default_statement_node(ASTNode * stmt) {
    ASTNode * node = create_ast();
    node->type = AST_DEFAULT_STMT;
    node->default_stmt.stmt = stmt;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_goto_statement(const char * label) {
    ASTNode * node = create_ast();
    node->type = AST_GOTO_STMT;
    node->goto_stmt.label = strdup(label);
    node->symbol = NULL;
    node->ctype = NULL;

    return node;
}

ASTNode * create_do_while_statement(ASTNode * stmt, ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_DO_WHILE_STMT;
    node->do_while_stmt.body = stmt;
    node->do_while_stmt.expr = expr;
    node->symbol = NULL;
    node->ctype = NULL;

    return node;
}

ASTNode * create_switch_statement(ASTNode * expr, ASTNode * stmt) {
    ASTNode * node = create_ast();
    node->type = AST_SWITCH_STMT;
    node->switch_stmt.expr = expr;
    node->switch_stmt.stmt = stmt;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_break_statement_node() {
    ASTNode * node = create_ast();
    node->type = AST_BREAK_STMT;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_continue_statement_node() {
    ASTNode * node = create_ast();
    node->type = AST_CONTINUE_STMT;
    node->symbol = NULL;
    node->ctype = NULL;
    return node;
}

ASTNode * create_int_literal_node(int value) {
    ASTNode * node = create_ast();
    node->type = AST_INT_LITERAL;
    node->int_value = value;
    node->ctype = &CTYPE_INT_T;
    node->symbol = NULL;
    return node;
}

ASTNode * create_function_declaration_node(const char * name, CType * returnType,
        ASTNode_list * param_list, CType * func_type, ASTNode * body, bool declaration_only) {
    ASTNode * func = create_ast();
    func->symbol = NULL;
    func->type = AST_FUNCTION_DECL;
    func->function_decl.name = strdup(name);
    func->ctype = returnType;
    func->function_decl.body = body;
    func->function_decl.param_list = param_list;
    func->function_decl.func_type = func_type;
    func->function_decl.declaration_only = declaration_only;
    if (func->function_decl.body != NULL) {
        func->function_decl.body->block.introduce_scope = false;
    }
    return func;
}

ASTNode * create_function_call_node(const char * name, ASTNode_list * args) {
    ASTNode * node = create_ast();
    node->type = AST_FUNCTION_CALL;
    node->function_call.name = strdup(name);
    node->function_call.arg_list = args;
    node->function_call.arg_count = (args != NULL) ? args->count : 0;
    node->ctype = NULL;
    return node;
}

ASTNode * create_var_decl_node(const char * name, CType * ctype, ASTNode * init_expr) {
    ASTNode * node = create_ast();
    node->type = AST_VAR_DECL;
    node->var_decl.name = strdup(name);
    node->ctype = ctype;
    node->var_decl.init_expr = init_expr;
    if (node->var_decl.init_expr) {
        node->var_decl.init_expr->ctype = ctype;
    }
    node->var_decl.is_param = false;
    node->var_decl.is_global = false;
    return node;
}


ASTNode * create_var_ref_node(const char * name) {
    ASTNode * node = create_ast();
    node->type = AST_VAR_REF;
    node->var_ref.name = strdup(name);
    node->ctype = NULL;
    return node;
}

ASTNode * create_array_access_node(ASTNode * arrayExpr, ASTNode * indexExpr) {
    ASTNode * node = create_ast();
    node->type = AST_ARRAY_ACCESS;
    node->array_access.base = arrayExpr;
    node->array_access.index = indexExpr;
    return node;
}

ASTNode * create_for_statement_node(ASTNode * init_expr, ASTNode * cond_expr,
            ASTNode * update_expr, ASTNode * body) {
    ASTNode * node = create_ast();
    node->type = AST_FOR_STMT;
    node->for_stmt.init_expr = init_expr;
    node->for_stmt.cond_expr = cond_expr;
    node->for_stmt.update_expr = update_expr;
    node->for_stmt.body = body;
    if (node->for_stmt.body != NULL) {
        node->for_stmt.body->block.introduce_scope = false;
    }
    return node;
}

ASTNode * create_return_statement_node(ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_RETURN_STMT;
    node->return_stmt.expr = expr;
    node->ctype = NULL;
    node->symbol = NULL;
    return node;
}

ASTNode * create_expression_statement_node(ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_EXPRESSION_STMT;
    node->expr_stmt.expr = expr;
    node->ctype = NULL;
    return node;
}

ASTNode * create_cast_expr_node(CType * target_type, ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_CAST_EXPR;
    node->cast_expr.target_type = target_type;
    node->cast_expr.expr = expr;
    node->ctype = NULL;
    return node;
}


ASTNode * create_block_node(ASTNode_list * stmts) {
    ASTNode * node = create_ast();
    node->type = AST_BLOCK;
    node->block.statements = stmts;
    node->block.count = stmts->count;
    node->block.introduce_scope = true;
    node->ctype = NULL;

    return node;
}

ASTNode * create_print_extension_node(ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_PRINT_EXTENSION_STATEMENT;
    node->expr_stmt.expr = expr;
    node->ctype = NULL;
    return node;
}

ASTNode * create_assert_extension_node(ASTNode * expr) {
    ASTNode * node = create_ast();
    node->type = AST_ASSERT_EXTENSION_STATEMENT;
    node->expr_stmt.expr = expr;
    node->ctype = NULL;
    return node;
}

ASTNode_list * create_node_list() {
    ASTNode_list * list = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(list, free_ast);

    return list;
}

CType * get_ctype_from_token(Token* token) {
    switch (token->type) {
        case TOKEN_INT: 
            return &CTYPE_INT_T;
            break;
        case TOKEN_CHAR:
            return &CTYPE_CHAR_T;
            break;
        case TOKEN_SHORT:
            return &CTYPE_SHORT_T;
            break;
        case TOKEN_LONG:
            return &CTYPE_LONG_T;
            break;
        default:
            error("Invalid TYPE Token\n");
            break;
    }
    return NULL;
}

