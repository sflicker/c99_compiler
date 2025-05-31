#include <stdlib.h>

#include "ast.h"

void free_astnode(ASTNode * node) {
    switch(node->type) {
        case AST_TRANSLATION_UNIT:
            ASTNode_list_free(node->translation_unit.functions);
            free(node->translation_unit.functions);
            break;

        case AST_VAR_DECL:
            free(node->var_decl.name);
            free_astnode(node->var_decl.init_expr);
        break;


        case AST_FUNCTION_DECL:
            free(node->function_decl.name);
            ASTNode_list_free(node->function_decl.param_list);
            free_astnode(node->function_decl.body);
            break;

        case AST_FUNCTION_CALL:
            ASTNode_list_free(node->function_call.arg_list);
            break;

        case AST_RETURN_STMT:
            free_astnode(node->return_stmt.expr);
            break;

        case AST_BLOCK:
            ASTNode_list_free(node->block.statements);
            free(node->block.statements);
            break;

        case AST_IF_STMT:
            free_astnode(node->if_stmt.cond);
            free_astnode(node->if_stmt.then_statement);
            free_astnode(node->if_stmt.else_statement);
            break;

        case AST_WHILE_STMT:
            free_astnode(node->while_stmt.cond);
            free_astnode(node->while_stmt.body);
            break;

        case AST_FOR_STMT:
            free_astnode(node->for_stmt.init_expr);
            free_astnode(node->for_stmt.cond_expr);
            free_astnode(node->for_stmt.update_expr);
            free_astnode(node->for_stmt.body);
            break;

        case AST_ASSERT_EXTENSION_STATEMENT:
        case AST_PRINT_EXTENSION_STATEMENT:
        case AST_EXPRESSION_STMT:
            free_astnode(node->expr_stmt.expr);
            break;

        case AST_ASSIGNMENT:
            free(node->assignment.name);
            free_astnode(node->assignment.expr);
            break;

        case AST_VAR_EXPR:
            free(node->var_expr.name);
            break;

        case AST_LABELED_STMT:
            free(node->labeled_stmt.label);
            free_astnode(node->labeled_stmt.stmt);
            break;

        case AST_SWITCH_STMT:
            free_astnode(node->switch_stmt.expr);
            free_astnode(node->switch_stmt.stmt);
            break;

        case AST_CASE_STMT:
            free(node->case_stmt.label);
            free_astnode(node->case_stmt.constExpression);
            free_astnode(node->case_stmt.stmt);
            break;

        case AST_DEFAULT_STMT:
            free_astnode(node->default_stmt.stmt);
            break;

        case AST_GOTO_STMT:
            free(node->goto_stmt.label);
            break;

        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD:
        case AST_EQUAL:
        case AST_NOT_EQUAL:
        case AST_LESS_THAN:
        case AST_LESS_EQUAL:
        case AST_GREATER_THAN:
        case AST_GREATER_EQUAL:
            free_astnode(node->binary.lhs);
            free_astnode(node->binary.rhs);
            break;

        case AST_UNARY_NEGATE:
        case AST_UNARY_PLUS:
        case AST_UNARY_NOT:
        case AST_UNARY_PRE_INC:
        case AST_UNARY_PRE_DEC:
        case AST_UNARY_POST_INC:
        case AST_UNARY_POST_DEC:
            free_astnode(node->unary.operand);            
            break;

        case AST_INT_LITERAL:
            // DO NOTHING
            break;

        default: 
            error("Invalid AST Node Type\n");
            break;
    }
    free(node);
}