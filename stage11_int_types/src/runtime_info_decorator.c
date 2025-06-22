#include "runtime_info_decorator.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "ast.h"
#include "emitter.h"
#include "token.h"
#include "ctypes.h"
#include "symbol_table.h"
#include "symbol.h"
#include "util.h"
#include "error.h"
#include "runtime_info.h"

RuntimeInfo_list * runtime_info_list = NULL;

void init_runtime_info_list() {
    runtime_info_list = malloc(sizeof(RuntimeInfo_list));
    RuntimeInfo_list_init(runtime_info_list, free_runtime_info);
}

void add_runtime_info_offset(ASTNode * astNode, int offset) {
    RuntimeInfo * runtimeNode = runtime_info(astNode);
    if (runtimeNode == NULL) {
        runtimeNode = malloc(sizeof(RuntimeInfo));
        runtimeNode->node = astNode;
        RuntimeInfo_list_append(runtime_info_list, runtimeNode);
    }
    runtimeNode->offset = offset;
}

RuntimeInfo * runtime_info(ASTNode * node) {
    for (RuntimeInfo_list_node * n = runtime_info_list->head; n != NULL; n = n->next) {
        RuntimeInfo * info = n->value;
        if (info->node == node) {
            return n->value;
        }
    }

    RuntimeInfo * runtimeNode = malloc(sizeof(RuntimeInfo));
    runtimeNode->node = node;
    RuntimeInfo_list_append(runtime_info_list, runtimeNode);
    return runtimeNode;
}

void populate_symbol_table(ASTNode * node) {

    if (!node) {
        return;
    }

    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {   
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                populate_symbol_table(n->value);
            }
            // for (int i=0;i<node->translation_unit.count;i++) {
            //     populate_symbol_table(node->translation_unit.functions[i], true);
            // }
            break;
        }
        case AST_FUNCTION_DECL:
            {
                // set_current_offset(0);
                // reset_storage_size();
                // enter_scope();
                // TypePtr_list * type_list = NULL;
                // if (node->function_decl.param_list && node->function_decl.param_list->count > 0) {
                //     type_list = malloc(sizeof(TypePtr_list));
                //     TypePtr_list_init(type_list, free_type);
                //
                //     int offset = 16;
                //     for (ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
                //         ASTNode * astNode = n->value;
                //         add_symbol(astNode->var_decl.name, astNode->var_decl.var_type);
                //         offset += astNode->var_decl.var_type->size;
                //         TypePtr_list_append(type_list, astNode->var_decl.var_type);
                //     }
                //     //for(int i=0;i<node->function_decl.param_count;i++) {
                //     //     ASTNode * astNode = node->function_decl.param_list[i];
                //     //     offset = add_symbol_with_offset(astNode->var_decl.name, offset, astNode->var_decl.var_type);
                //     //     astNode->var_decl.offset = offset;
                //     //     offset += astNode->var_decl.var_type->size;
                //     //     //offset += 8;
                //     // }
                // }
                // //Type * param_type;
                // add_function_symbol(node->function_decl.name, node->function_decl.return_type,
                //      ((node->function_decl.param_list != NULL) ? node->function_decl.param_list->count : 0), type_list);
                // populate_symbol_table(node->function_decl.body, false);
                // node->function_decl.size = get_symbol_total_space();
                // exit_scope();
                break;
            }
        // case AST_PARAM_LIST: {
        //     // for (struct node_list * node_list = node->param_list.node_list; node_list != NULL; node_list = node_list->next) {
        //     //     populate_symbol_table(node_list->node, false);
        //     // }
        //     break;
        // }
        case AST_BLOCK:
            if (node->block.introduce_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n; n = n->next) {
                populate_symbol_table(n->value);
            }

            // for (int i=0;i<node->block.count;i++) {
            //     populate_symbol_table(node->block.statements[i], true);
            // }
            
            if (node->block.introduce_scope) exit_scope();
            break;
        case AST_VAR_DECL:
//            Symbol * symbol = add_symbol(node->var_decl.name, node->ctype, node);
            Symbol * symbol = node->symbol;
            int offset = 0;     // TODO FIX
            symbol->info.var.offset = offset;
            //runtime_info(node)->offset = offset;
//            add_runtime_info(node, offset);
//            node->var_decl.addr = offset;
            if (node->var_decl.init_expr) {
                populate_symbol_table(node->var_decl.init_expr);
            }
            break;
//         case AST_ASSIGNMENT:
//         case AST_COMPOUND_ADD_ASSIGN:
//         case AST_COMPOUND_SUB_ASSIGN: {
// //            add_symbol(node->assignment.name);
//             Address addr = lookup_symbol(node->assignment.name);
//             node->assignment.offset = offset;
//             populate_symbol_table(node->assignment.expr);
//             break;
//         }
        case AST_RETURN_STMT:
            populate_symbol_table(node->return_stmt.expr);
            break;

        case AST_IF_STMT:
            populate_symbol_table(node->if_stmt.cond);
            populate_symbol_table(node->if_stmt.then_stmt);
            if (node->if_stmt.else_stmt) {
                populate_symbol_table(node->if_stmt.else_stmt);
            }
            break;

        case AST_EXPRESSION_STMT:
            populate_symbol_table(node->expr_stmt.expr);
            break;

        // case AST_UNARY_POST_INC:
        // case AST_UNARY_POST_DEC:
        // case AST_UNARY_PRE_INC:
        // case AST_UNARY_PRE_DEC:
        // case AST_UNARY_NEGATE:
        // case AST_UNARY_NOT:
        // case AST_UNARY_PLUS:
        case AST_UNARY_EXPR:
            populate_symbol_table(node->unary.operand);
            break;
        case AST_VAR_REF: {
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            // TODO
            runtime_info(node)->offset = runtime_info(symbol->node)->offset;
            //node->var_ref.addr = offset;
            break;
        }

        case AST_WHILE_STMT:
            populate_symbol_table(node->while_stmt.cond);
            populate_symbol_table(node->while_stmt.body);

        case AST_FOR_STMT:
            enter_scope();
            populate_symbol_table(node->for_stmt.init_expr);
            populate_symbol_table(node->for_stmt.cond_expr);
            populate_symbol_table(node->for_stmt.update_expr);
            populate_symbol_table(node->for_stmt.body);
            exit_scope();
            break;


        // case AST_LESS_THAN:
        // case AST_LESS_EQUAL:
        // case AST_GREATER_THAN:
        // case AST_GREATER_EQUAL:
        // case AST_EQUAL:
        // case AST_NOT_EQUAL:
        // case AST_DIV:
        // case AST_MOD:
        // case AST_MUL:
        // case AST_ADD:
        // case AST_SUB:
        // case AST_LOGICAL_AND:
        // case AST_LOGICAL_OR:
        case AST_BINARY_EXPR:
            populate_symbol_table(node->binary.lhs);
            populate_symbol_table(node->binary.rhs);
            break;

        case AST_FUNCTION_CALL: {
            for (ASTNode_list_node *n = node->function_call.arg_list->head; n; n=n->next) {
                populate_symbol_table(n->value);
            }
            // for (struct node_list * curr = node->function_call.argument_expression_list; curr != NULL; curr = curr->next) {
            //     ASTNode * node = (ASTNode*)curr->node;
            //     populate_symbol_table(node, false);
            // }
            break;
        }
        // case AST_ARG_LIST:
        //     // TODO
        //     break;

        case AST_INT_LITERAL:
            // DO NOTHING
            break;

        case AST_SWITCH_STMT:
            populate_symbol_table(node->switch_stmt.expr);
            populate_symbol_table(node->switch_stmt.stmt);
            break;

        case AST_ASSERT_EXTENSION_STATEMENT:
            populate_symbol_table(node->expr_stmt.expr);
            break;

        case AST_PRINT_EXTENSION_STATEMENT:
            populate_symbol_table(node->expr_stmt.expr);
            break;
        case AST_GOTO_STMT:         //TODO make need implementation
        case AST_CASE_STMT:
        case AST_DEFAULT_STMT:
        case AST_LABELED_STMT:
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
            break;

        case AST_DO_WHILE_STMT:
            populate_symbol_table(node->do_while_stmt.expr);
            populate_symbol_table(node->do_while_stmt.body);
            break;

        default:
            error("Unknown AST Node Type: %d\n", node->type);

    }
}

