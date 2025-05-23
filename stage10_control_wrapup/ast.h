#ifndef AST_H
#define AST_H

#include <stdbool.h>

#include "token.h"
#include "util.h"

typedef enum {
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_LABELED_STMT,
    AST_CASE_STMT,
    AST_DEFAULT_STMT,
    AST_GOTO_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,
    AST_PARAM_LIST,
    AST_ARG_LIST,
    AST_TRANSLATION_UNIT,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_VAR_EXPR,
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
    AST_MOD,
    AST_EQUAL,
    AST_NOT_EQUAL,
    AST_LOGICAL_AND,
    AST_LOGICAL_OR,
    AST_LESS_THAN,
    AST_LESS_EQUAL,
    AST_GREATER_THAN,
    AST_GREATER_EQUAL,
    AST_ASSERT_EXTENSION_STATEMENT,
    AST_PRINT_EXTENSION_STATEMENT
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
            struct ASTNode* body;
            bool declaration_only;
            int size;
        } function_decl;

        struct {
            struct node_list * node_list;
        } param_list;

        struct {
            const char * name;
            struct node_list * argument_expression_list;
        } function_call;

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
            int offset;
        } var_decl;

        struct {
            const char * name;
            int offset;
            struct ASTNode * expr;
        } assignment;

        struct {
            const char * name;
            int offset;
        } var_expr;

        struct {
            const char * label;
            struct ASTNode * stmt;
        } labeled_stmt;

        struct {
            struct ASTNode * constExpression;
            struct ASTNode * stmt;
        } case_stmt;

        struct {
            struct ASTNode * stmt;
        } default_stmt;

        struct {
            const char * label;
        } goto_stmt;
    };
} ASTNode;

#endif