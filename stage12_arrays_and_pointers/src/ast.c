#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "ast.h"

void free_astnode(ASTNode * node) {
    if (!node) return;
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
            if (node->function_decl.param_list != NULL) {
                ASTNode_list_free(node->function_decl.param_list);
                free(node->function_decl.param_list);
            }
            free_astnode(node->function_decl.body);
            break;

        case AST_FUNCTION_CALL:
            ASTNode_list_free(node->function_call.arg_list);
            free(node->function_call.arg_list);
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
            free_astnode(node->if_stmt.then_stmt);
            free_astnode(node->if_stmt.else_stmt);
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

        case AST_VAR_REF:
            free(node->var_ref.name);
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
            free_astnode(node->case_stmt.constExpression);
            free_astnode(node->case_stmt.stmt);
            break;

        case AST_DEFAULT_STMT:
            free_astnode(node->default_stmt.stmt);
            break;

        case AST_GOTO_STMT:
            free(node->goto_stmt.label);
            break;

        case AST_BINARY_EXPR:
            free_astnode(node->binary.lhs);
            free_astnode(node->binary.rhs);
            break;

        case AST_UNARY_EXPR:
            free_astnode(node->unary.operand);            
            break;

        case AST_DO_WHILE_STMT:
            free_astnode(node->do_while_stmt.expr);
            free_astnode(node->do_while_stmt.body);
            break;

        case AST_CAST_EXPR:
            free_astnode(node->cast_expr.expr);
            break;

        case AST_CONTINUE_STMT:
        case AST_BREAK_STMT:
        case AST_INT_LITERAL:
            // DO NOTHING
            break;

        default: 
            error("Invalid AST Node Type: %d\n", node->type);
            break;
    }
    free(node);
}

BinaryOperator get_binary_operator_from_tok(Token * tok) {
    switch (tok->type) {
        case TOKEN_PLUS: return BINOP_ADD; break;
        case TOKEN_MINUS: return BINOP_SUB; break;
        case TOKEN_STAR: return BINOP_MUL; break;
        case TOKEN_DIV: return BINOP_DIV; break;
        case TOKEN_PERCENT: return BINOP_MOD; break;
        case TOKEN_GT: return BINOP_GT; break;
        case TOKEN_GE: return BINOP_GE; break;
        case TOKEN_LT: return BINOP_LT; break;
        case TOKEN_LE: return BINOP_LE; break;
        default: return BINOP_UNASSIGNED_OP; break;
    }

}

const char * get_binary_op_name(BinaryOperator op) {
    switch (op) {
        case BINOP_ADD: return "ADD"; break;
        case BINOP_SUB: return "SUB"; break;
        case BINOP_MUL: return "MUL"; break;
        case BINOP_DIV: return "DIV"; break;
        case BINOP_MOD: return "MOD"; break;
        case BINOP_EQ: return "EQ"; break;
        case BINOP_NE: return "NE"; break;
        case BINOP_GT: return "GT"; break;
        case BINOP_GE: return "GE"; break;
        case BINOP_LT: return "LT"; break;
        case BINOP_LE: return "LE"; break;
        case BINOP_LOGICAL_OR: return "OR"; break;
        case BINOP_LOGICAL_AND: return "AND"; break;
        case BINOP_ASSIGNMENT: return "ASN"; break;
        case BINOP_COMPOUND_ADD_ASSIGN: return "ADDASN"; break;
        case BINOP_COMPOUND_SUB_ASSIGN: return "SUBASN"; break;
        default:
            return "UNASSIGNED"; break;
    }
    return NULL;
}

const char * get_unary_op_name(UnaryOperator op) {
    switch(op) {
        case UNARY_NEGATE: return "NEGATE"; break;
        case UNARY_PLUS: return "PLUS"; break;
        case UNARY_NOT: return "NOT"; break;
        case UNARY_PRE_INC: return "PRE_INC"; break;
        case UNARY_PRE_DEC: return "PRE_DEC"; break;
        case UNARY_POST_INC: return "POST_INC"; break;
        case UNARY_POST_DEC: return "POST_DEC"; break;
        case UNARY_UNASSIGNED_OP: return "UNASSIGNED"; break;
    }
    return NULL;
}

bool binop_equal(BinaryOperator a, BinaryOperator b) {
    if (a != b) {
        fprintf(stderr, "binop do not match %s, %s",
            get_binary_op_name(a), get_binary_op_name(b));
        return false;
    }
    return true;
}

bool ast_equal(ASTNode * a, ASTNode * b) {
    if (!a || !b) return a == b;

    if (a->type != b->type) {
        fprintf(stderr, "ast node types do not match %d, %d", a->type, b->type);
        return false;
    }
    if (!ctype_equals(a->ctype, b->ctype)) {
        fprintf(stderr, "ast node ctypes do not match %s, %s",
            ctype_to_string(a->ctype), ctype_to_string(b->ctype));
        return false;
    }
    switch (a->type) {
        case AST_VAR_DECL:
            if (!strcmp(a->var_decl.name, b->var_decl.name) == 0) {
                fprintf(stderr, "ast variable names do not match - %s, %s\n",
                    a->var_decl.name, a->var_decl.name);
                return false;
            }
            return true;
            break;
        case AST_VAR_REF:
            if (!strcmp(a->var_ref.name, b->var_ref.name) == 0) {
                fprintf(stderr, "ast variable names do not match - %s, %s\n",
                    a->var_ref.name, b->var_ref.name);
                return false;
            }
            return true;
            break;
        case AST_INT_LITERAL:
            return a->int_value == b->int_value;
            break;
        case AST_BINARY_EXPR:
            return (binop_equal(a->binary.op, b->binary.op)) &&
                ast_equal(a->binary.lhs, b->binary.lhs ) &&
                ast_equal(a->binary.rhs, b->binary.rhs);
        case AST_UNARY_EXPR:
            return (a->unary.op == b->unary.op) &&
                ast_equal(a->unary.operand, b->unary.operand);
        case AST_FUNCTION_CALL:
            return (strcmp(a->function_call.name, b->function_call.name) == 0 &&
                ctype_lists_equal(
                    astNodeListToTypeList(a->function_call.arg_list),
                    astNodeListToTypeList(b->function_call.arg_list)
                )
            );
        case AST_EXPRESSION_STMT:
        case AST_RETURN_STMT:
            return ast_equal(a->return_stmt.expr, b->return_stmt.expr);
            break;
        default:
            error("Invalid AST Node Type: %d\n", a->type);
    }
    fprintf(stderr, "ASTNodes are not equal\n");
    return false;
}