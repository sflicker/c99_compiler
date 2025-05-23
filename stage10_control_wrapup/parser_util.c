#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "token.h"
#include "ast.h"
#include "parser_context.h"
#include "parser_util.h"

ASTNode * create_unary_node(ASTNodeType op, ASTNode * operand) {
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = op;
    node->unary.operand = operand;
    return node;
}

ASTNodeType binary_op_token_to_ast_type(TokenType tok) {
    switch (tok) {
        case TOKEN_PLUS: return AST_ADD;
        case TOKEN_MINUS: return AST_SUB;
        case TOKEN_STAR: return AST_MUL;
        case TOKEN_DIV: return AST_DIV;
        case TOKEN_PERCENT: return AST_MOD;
        case TOKEN_EQ: return AST_EQUAL;
        case TOKEN_NEQ: return AST_NOT_EQUAL;
        case TOKEN_LT: return AST_LESS_THAN;
        case TOKEN_LE: return AST_LESS_EQUAL;
        case TOKEN_GT: return AST_GREATER_THAN;
        case TOKEN_GE: return AST_GREATER_EQUAL;
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
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_IF_STMT;
    node->if_stmt.cond = condExpression;
    node->if_stmt.then_statement = thenStatement;
    node->if_stmt.else_statement = elseStatement;
    return node;
}

ASTNode * create_while_statement_node(ASTNode * condExpression, ASTNode * bodyStatement) {
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STMT;
    node->while_stmt.cond = condExpression;
    node->while_stmt.body = bodyStatement;
    return node;
}

bool is_lvalue(ASTNode * node) {
    return node->type == AST_VAR_EXPR;
}