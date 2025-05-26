#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "emitter.h"
#include "token.h"
#include "symtab.h"
#include "util.h"


void populate_symbol_table(ASTNode * node, bool make_new_scope) {

    if (!node) {
        return;
    }

    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {   
            for (int i=0;i<node->translation_unit.count;i++) {
                populate_symbol_table(node->translation_unit.functions[i], true);
            }
            break;
        }
        case AST_FUNCTION_DECL:
            {
                set_current_offset(0);
                reset_storage_size();
                enter_scope();
                if (node->function_decl.param_list) {
                    int offset = 16;
                    for(struct node_list * curr = node->function_decl.param_list->param_list.node_list; curr != NULL; curr = curr->next) {
                        ASTNode * astNode = (ASTNode*)curr->node;
                        offset = add_symbol_with_offset(astNode->var_decl.name, offset);
                        astNode->var_decl.offset = offset;
                        offset += 8;
                    }
                }
                populate_symbol_table(node->function_decl.body, false);
                node->function_decl.size = get_symbol_total_space();
                exit_scope();
                break;
            }
        case AST_PARAM_LIST: {
            for (struct node_list * node_list = node->param_list.node_list; node_list != NULL; node_list = node_list->next) {
                populate_symbol_table(node_list->node, false);
            }
            break;
        }
        case AST_BLOCK:
            if (make_new_scope) enter_scope();

            for (int i=0;i<node->block.count;i++) {
                populate_symbol_table(node->block.statements[i], true);
            }
            
            if (make_new_scope) exit_scope();
            break;
        case AST_VAR_DECL:
            int offset = add_symbol(node->var_decl.name);
            node->var_decl.offset = offset;
            if (node->var_decl.init_expr) {
                populate_symbol_table(node->var_decl.init_expr, false);
            }
            break;
        case AST_ASSIGNMENT:
        case AST_COMPOUND_ADD_ASSIGN:
        case AST_COMPOUND_SUB_ASSIGN: {
//            add_symbol(node->assignment.name);
            int offset = lookup_symbol(node->assignment.name);
            node->assignment.offset = offset;
            populate_symbol_table(node->assignment.expr, false);
            break;
        }
        case AST_RETURN_STMT:
            populate_symbol_table(node->return_stmt.expr, true);
            break;

        case AST_IF_STMT:
            populate_symbol_table(node->if_stmt.cond, true);
            populate_symbol_table(node->if_stmt.then_statement, true);
            if (node->if_stmt.else_statement) {
                populate_symbol_table(node->if_stmt.else_statement, true);
            }
            break;

        case AST_EXPRESSION_STMT:
            populate_symbol_table(node->expr_stmt.expr, true);
            break;

        case AST_UNARY_POST_INC:
        case AST_UNARY_POST_DEC:
        case AST_UNARY_PRE_INC:
        case AST_UNARY_PRE_DEC:
        case AST_UNARY_NEGATE:
        case AST_UNARY_NOT:
        case AST_UNARY_PLUS:
            populate_symbol_table(node->unary.operand, true);
            break;
        case AST_VAR_EXPR: {
            int offset = lookup_symbol(node->var_expr.name);
            node->var_expr.offset = offset;
            break;
        }

        case AST_WHILE_STMT:
            populate_symbol_table(node->while_stmt.cond, true);
            populate_symbol_table(node->while_stmt.body, true);

        case AST_FOR_STMT:
            enter_scope();
            populate_symbol_table(node->for_stmt.init_expr, false);
            populate_symbol_table(node->for_stmt.cond_expr, false);
            populate_symbol_table(node->for_stmt.update_expr, false);
            populate_symbol_table(node->for_stmt.body , false);
            exit_scope();
            break;

        case AST_LESS_THAN:
        case AST_LESS_EQUAL:
        case AST_GREATER_THAN:
        case AST_GREATER_EQUAL:
        case AST_EQUAL:
        case AST_NOT_EQUAL:
        case AST_DIV:
        case AST_MOD:
        case AST_MUL:
        case AST_ADD:
        case AST_SUB:
        case AST_LOGICAL_AND:
        case AST_LOGICAL_OR:
            populate_symbol_table(node->binary.lhs, true);
            populate_symbol_table(node->binary.rhs, true);
            break;

        case AST_FUNCTION_CALL: {
            for (struct node_list * curr = node->function_call.argument_expression_list; curr != NULL; curr = curr->next) {
                ASTNode * node = (ASTNode*)curr->node;
                populate_symbol_table(node, false);
            }
            break;
        }
        case AST_ARG_LIST:
            // TODO
            break;

        case AST_INT_LITERAL:
            // DO NOTHING
            break;

        case AST_SWITCH_STMT:
            populate_symbol_table(node->switch_stmt.expr, true);
            populate_symbol_table(node->switch_stmt.stmt, true);
            break;

        case AST_ASSERT_EXTENSION_STATEMENT:
            populate_symbol_table(node->expr_stmt.expr, true);
            break;

        case AST_PRINT_EXTENSION_STATEMENT:
            populate_symbol_table(node->expr_stmt.expr, true);
            break;
        case AST_GOTO_STMT:         //TODO make need implementation
        case AST_CASE_STMT:
        case AST_DEFAULT_STMT:
        case AST_LABELED_STMT:
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
            break;

        case AST_DO_WHILE_STMT:
            populate_symbol_table(node->do_while_stmt.expr, true);
            populate_symbol_table(node->do_while_stmt.stmt, true);
            break;

        default:
            error("Unknown AST Node Type: %d\n", node->type);

    }
}

