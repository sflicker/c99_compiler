#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,
    AST_TRANSLATION_UNIT,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_VAR_EXPR,
    AST_PARAM_LIST,
    AST_ARGUMENT_EXPRESSION_LIST,
    AST_ASSIGNMENT,
    AST_COMPOUND_ADD_ASSIGN,
    AST_COMPOUND_SUB_ASSIGN,
    AST_UNARY_POST_INC,
    AST_UNARY_POST_DEC,
    AST_UNARY_PRE_INC,
    AST_UNARY_PRE_DEC,
    AST_UNARY_NEGATE,
    AST_UNARY_NOT,
    AST_UNARY_PLUS,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_EQUAL,
    AST_NOT_EQUAL,
    AST_LOGICAL_AND,
    AST_LOGICAL_OR,
    AST_LESS_THAN,
    AST_LESS_EQUAL,
    AST_GREATER_THAN,
    AST_GREATER_EQUAL,
    AST_ASSERT_STATEMENT
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;

        struct {
            struct ASTNode ** functions;
            int count;
        } translation_unit;

        struct {
            const char* name;
            struct ASTNode* param_list;
            int num_params;
            struct ASTNode* body;
            bool declaration_only;
        } function_decl;

        struct {
            struct ASTNode * param;
            struct ASTNode * next;
        } param_list;

        struct {
            const char * name;
            struct ASTNode * argument_expression_list;
            int num_args;
        } function_call;

        struct {
            struct ASTNode * expression;
            struct ASTNode * next;
        } argument_list;

        struct {
            struct ASTNode * expr;
        } return_stmt;

        struct {
            struct ASTNode * lhs;
            struct ASTNode * rhs;
        } binary;

        struct {
            struct ASTNode * operand;
        } unary;

        struct {
            struct ASTNode ** statements;
            int count;
            int capacity;
        } block;

        struct {
            struct ASTNode * cond;
            struct ASTNode * then_statement;
            struct ASTNode * else_statement;
        } if_stmt;

        struct {
            struct ASTNode * cond;
            struct ASTNode * body;
        } while_stmt;

        struct {
            struct ASTNode * init_expr;
            struct ASTNode * cond_expr;
            struct ASTNode * update_expr;
            struct ASTNode * body;
        } for_stmt;

        struct {
            struct ASTNode * expr;
        } expr_stmt;

        struct {
            const char* name;
            struct ASTNode * init_expr; // NULL if no initializer
        } var_decl;

        struct {
            const char * name;
            struct ASTNode * expr;
        } assignment;

        struct {
            const char * name;
        } var_expr;

    };
} ASTNode;

#endif