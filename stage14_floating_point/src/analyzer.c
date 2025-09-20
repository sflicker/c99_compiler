//
// Created by scott on 6/14/25.
//

#include <assert.h>

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "c_type.h"
#include "parser_util.h"
#include "symbol.h"
#include "symbol_table.h"

//int local_offset = -8;
int local_offset = 0;
int param_offset = 16;
int function_local_storage = 0;

void reset_size_and_offsets() {
    local_offset = 0;
    param_offset = 16;
    function_local_storage = 0;
}

CType * apply_integer_promotions(CType * t) {
    if (t->kind == CTYPE_CHAR || t->kind == CTYPE_SHORT) {
        return &CTYPE_INT_T;
    }
    return t;
}

CType * usual_arithmetic_conversion(CType * a, CType * b) {
    if (a == b) return a;
    if (a->rank > b->rank) return a;
    return b;
}

bool is_lvalue(ASTNode * node) {
    if (node == NULL) {
        error("node must not be null");
    }
    switch (node->type) {
        case AST_VAR_REF_EXPR:
        case AST_ARRAY_ACCESS:
            return true;

        case AST_UNARY_EXPR:
            if (node->unary.op == UNARY_DEREF) {
                return true;
            }

        default:
            return false;
    }
}


int get_ast_node_list_length(ASTNode_list * ast_nodes) {
    return (ast_nodes != NULL) ? ast_nodes->count : 0;
}

void handle_function_definition(AnalyzerContext *ctx, ASTNode * node) {
    enter_scope();
    reset_size_and_offsets();
    Symbol_list * symbol_list = NULL;
    if (node->function_def.param_list != NULL) {
        symbol_list = malloc(sizeof(Symbol_list));
        Symbol_list_init(symbol_list, free_symbol);;
        for (ASTNode_list_node * n = node->function_def.param_list->head; n != NULL; n = n->next) {
            const char * name = n->value->var_decl.name;
            ASTNode * param = n->value;

            Symbol * symbol = create_storage_param_symbol(name, param, param->ctype, &param_offset);

            add_symbol(symbol);

            param->symbol = symbol;
            Symbol_list_append(symbol_list, symbol);
        }
    }
    Symbol * symbol = create_symbol(node->function_def.name, SYMBOL_FUNC, node->ctype, node);
    symbol->info.func.num_params = get_ast_node_list_length(node->function_def.param_list);
    symbol->info.func.params_symbol_list = symbol_list;
    add_global_symbol(symbol);
    node->symbol = symbol;

    CType * saved = ctx->current_function_return_type;
    ctx->current_function_return_type = node->ctype;
    analyze(ctx, node->function_def.body);
    ctx->current_function_return_type = saved;
    node->function_def.size = function_local_storage;
    exit_scope();

}

void handle_function_declaration(AnalyzerContext * ctx, ASTNode * node) {

    // enter_scope();
    // reset_size_and_offsets();
    Symbol_list * symbol_list = NULL;
    if (node->function_decl.param_list != NULL) {
        symbol_list = malloc(sizeof(Symbol_list));
        Symbol_list_init(symbol_list, free_symbol);;
        for (ASTNode_list_node * n = node->function_decl.param_list->head; n != NULL; n = n->next) {
            const char * name = n->value->var_decl.name;
            ASTNode * param = n->value;

            Symbol * symbol = create_storage_param_symbol(name, param, param->ctype, &param_offset);

//            add_symbol(symbol);

            param->symbol = symbol;
            Symbol_list_append(symbol_list, symbol);
        }
    }
    Symbol * symbol = create_symbol(node->function_decl.name, SYMBOL_FUNC, node->ctype, node);
    symbol->info.func.num_params = get_ast_node_list_length(node->function_decl.param_list);
    symbol->info.func.params_symbol_list = symbol_list;
    add_global_symbol(symbol);
    node->symbol = symbol;

//     CType * saved = ctx->current_function_return_type;
//     ctx->current_function_return_type = node->ctype;
// //    analyze(ctx, node->function_decl.body);
//     ctx->current_function_return_type = saved;
//     node->function_decl.size = function_local_storage;
//     exit_scope();
}



CType * get_binary_expr_return_type(CType * common, BinaryOperator op) {
    if (is_comparison_op(op)) {
        return  &CTYPE_INT_T;
    }
    return common;
}

void verify_expr(AnalyzerContext * ctx, ASTNode * node) {
    if (!node) return;

    switch (node->type) {
        case AST_VAR_REF_EXPR: assert(node->ctype); break;
        case AST_INT_LITERAL: assert(node->ctype); break;
        case AST_FLOAT_LITERAL: assert(node->ctype); break;
        case AST_DOUBLE_LITERAL: assert(node->ctype); break;
        case AST_UNARY_EXPR: assert(node->ctype); break;
        case AST_BINARY_EXPR: {
            if (!is_assignment(node)) {
                assert(node->ctype);
                assert(node->binary.common_type);
            }
            break;
        }

        case AST_FUNCTION_CALL_EXPR: {
            assert(node->ctype);
            if (node->function_call.arg_list != NULL) {
                for (ASTNode_list_node * arg = node->function_call.arg_list->head; arg != NULL; arg = arg->next) {
                    verify_expr(ctx, arg->value);
                }
            }
            break;
        }

        case AST_EXPRESSION_STMT:
            verify_expr(ctx, node->expr_stmt.expr);
            break;

        case AST_IF_STMT:
            verify_expr(ctx, node->if_stmt.cond);
            verify_expr(ctx, node->if_stmt.then_stmt);
            verify_expr(ctx, node->if_stmt.else_stmt);

        case AST_BLOCK_STMT: {
            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                verify_expr(ctx, n->value);
            }
            break;
        }

        case AST_WHILE_STMT: {
            verify_expr(ctx, node->while_stmt.cond);
            verify_expr(ctx, node->while_stmt.body);
            break;
        }

        case AST_FOR_STMT: {
            verify_expr(ctx, node->for_stmt.init_expr);
            verify_expr(ctx, node->for_stmt.cond_expr);
            verify_expr(ctx, node->for_stmt.update_expr);
            verify_expr(ctx, node->for_stmt.body);
            break;
        }

        case AST_DO_WHILE_STMT:
            verify_expr(ctx, node->do_while_stmt.expr);
            verify_expr(ctx, node->do_while_stmt.body);
            break;

        case AST_TRANSLATION_UNIT:
        case AST_INITIALIZER_LIST:
        case AST_GOTO_STMT:
        case AST_LABELED_STMT:
            break;

        case AST_CASE_STMT:
            verify_expr(ctx, node->case_stmt.constExpression);
            verify_expr(ctx, node->case_stmt.stmt);
            break;

        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
        case AST_SWITCH_STMT:
        case AST_DEFAULT_STMT:
            // no check
            break;

        case AST_FUNCTION_DEF:
        case AST_FUNCTION_DECL:
            // TODO add verifications (probably different)
            break;

        case AST_CAST_EXPR:
            assert(node->cast_expr.expr->ctype);
            assert(node->cast_expr.target_ctype);
            assert(node->ctype);
            break;

        case AST_RETURN_STMT:
            verify_expr(ctx, node->return_stmt.expr);
            assert(ctype_equals(node->ctype, ctx->current_function_return_type));
            break;

        case AST_DECLARATION_STMT: {
            for (ASTNode_list_node * n = node->declaration.init_declarator_list->head; n; n = n->next) {
                assert(n->value->ctype);
            }
        }
        break;

        case AST_VAR_DECL:
            assert(node->symbol);
            assert(node->symbol->ctype);
            assert(node->ctype);
            break;

        case AST_ARRAY_ACCESS:
            assert(node->ctype);
            assert(node->array_access.base->ctype);
            assert(node->array_access.index->ctype);
            break;

        default: error("Not Currently Handling ASTNode type %d", get_ast_node_name(node));
            break;
    }
}

void set_element_type(AnalyzerContext * ctx, ASTNode * node, CType * ctype) {
    assert(node->type == AST_INITIALIZER_LIST);
    node->initializer_list.element_type = ctype;
    for (ASTNode_list_node * n = node->initializer_list.items->head; n != NULL; n = n->next) {
        if (n->value->type == AST_INITIALIZER_LIST) {
            set_element_type(ctx, n->value, ctype);
        }
    }
}

void analyze(AnalyzerContext * ctx, ASTNode * node) {
    if (!node) return;

    switch (node->type) {
        case AST_TRANSLATION_UNIT: {
            setTranslationUnit(ctx, node);
            enter_scope();
            for (ASTNode_list_node * n = node->translation_unit.globals->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                analyze(ctx, n->value);
            }
            exit_scope();
            break;
        }
        case AST_FUNCTION_DECL: {
            handle_function_declaration(ctx, node);
            break;
        }

        case AST_FUNCTION_DEF: {
            handle_function_definition(ctx, node);
            break;
        }

        case AST_FUNCTION_CALL_EXPR: {
            Symbol * functionSymbol = lookup_table_symbol(getGlobalScope(), node->function_call.name);
            if (!functionSymbol) {
                error("function symbol not found - %s", node->function_call.name);
                return;
            }

            size_t arg_index = 0;
            if (node->function_call.arg_list != NULL) {
                for (ASTNode_list_node * arg = node->function_call.arg_list->head; arg != NULL; arg = arg->next) {
                    analyze(ctx, arg->value);
                    CType * arg_type = arg->value->ctype;

                    if (arg_index >= functionSymbol->info.func.num_params) {
                        error("Too many arguments for function %s", node->function_call.name);
                        return;
                    }

                    if (is_array_type(arg_type)) {
                        arg_type = make_pointer_type(arg_type->base_type);
                    }
                    CType * param_type = Symbol_list_get(functionSymbol->info.func.params_symbol_list, arg_index)->ctype;
                    if (!ctype_equal_or_compatible( param_type, arg_type)) {
                        if (is_castable(param_type, arg_type)) {
                            arg->value = create_cast_expr_node(param_type, arg->value);
                        }
                        else {
                            warning("Type mismatch for function %s", node->function_call.name);
                            return;
                        }
                    }

                    arg_index++;
                }
            }

            if (arg_index < functionSymbol->info.func.num_params) {
                error("Too few arguments to function %s", node->function_call.name);
                return;
            }

            node->symbol = functionSymbol;
            node->ctype = functionSymbol->node->ctype;
            break;
        }

        case AST_BLOCK_STMT:
            if (node->block.introduce_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }

            if (node->block.introduce_scope) exit_scope();
            break;

        case AST_VAR_DECL:
            Symbol * symbol = create_symbol(node->var_decl.name, SYMBOL_VAR, node->ctype, node);
            if (node->var_decl.is_param) {
                symbol->info.var.storage = STORAGE_PARAMETER;
                // handled in function
                //symbol->info.var.offset = param_offset;
                //symbol->info.var.storage = STORAGE_PARAMETER;
            }
            else if (node->var_decl.is_global) {
                symbol->info.var.storage = STORAGE_GLOBAL;
                symbol->storage = STORAGE_GLOBAL;
            }
            else {
                symbol->info.var.offset = local_offset - node->ctype->size;
                symbol->info.var.storage = STORAGE_LOCAL;
                symbol->storage = STORAGE_LOCAL;
                // local_offset -= 8;
                // function_local_storage += 8;
                local_offset -= node->ctype->size;
                function_local_storage += node->ctype->size;

            }
            add_symbol(symbol);
            node->symbol = symbol;
            if (node->var_decl.init_expr) {
                if (node->var_decl.init_expr->type == AST_INITIALIZER_LIST) {
                    CType * element_type = NULL;
                    if (is_array_type(node->ctype)) {
                        element_type = get_base_type(node->ctype);
                    } else {
                        element_type = node->ctype;
                    }
                    node->var_decl.init_expr->ctype = node->ctype;
                    set_element_type(ctx, node->var_decl.init_expr, element_type);

                }
                analyze(ctx, node->var_decl.init_expr);

                if (node->var_decl.init_expr->type == AST_FUNCTION_CALL_EXPR) {
                    //TODO
                } else if (!ctype_equals(node->ctype, node->var_decl.init_expr->ctype)) {
                    node->var_decl.init_expr =
                        create_cast_expr_node(node->ctype, node->var_decl.init_expr);
                }
            }
            break;

        case AST_BINARY_EXPR:
            analyze(ctx, node->binary.lhs);
            analyze(ctx, node->binary.rhs);

            if (is_assignment(node)) {
                if (!is_lvalue(node->binary.lhs)) {
                    error("Assignment must be to an lvalue");
                }
                else {
                    if (node->binary.lhs->ctype != node->binary.rhs->ctype) {
                        if (is_castable(node->binary.lhs->ctype, node->binary.rhs->ctype)) {
                            node->binary.rhs = create_cast_expr_node(node->binary.lhs->ctype, node->binary.rhs);
                        }
                    }
                    node->binary.common_type = node->binary.lhs->ctype;
                }
            }
            else {
                CType * lhsCType = node->binary.lhs->ctype;
                CType * rhsCType = node->binary.rhs->ctype;

                if (is_floating_point_type(lhsCType) || is_floating_point_type(rhsCType)) {
                    if (!ctype_equals(lhsCType, rhsCType)) {
                        if (lhsCType->rank > rhsCType->rank) {
                            node->binary.rhs = create_cast_expr_node(lhsCType, node->binary.rhs);
                            node->ctype = get_binary_expr_return_type(lhsCType, node->binary.op);
                            node->binary.common_type = lhsCType;
                        }
                        else if (lhsCType->rank < rhsCType->rank) {
                            node->binary.lhs = create_cast_expr_node(rhsCType, node->binary.lhs);
                            node->ctype = get_binary_expr_return_type(rhsCType, node->binary.op);
                            node->binary.common_type = rhsCType;
                        }
                    }
                    else {
                        node->ctype = get_binary_expr_return_type(lhsCType, node->binary.op);
                        node->binary.common_type = lhsCType;
                    }
                }
                else {
                    CType * promoted_left = is_integer_type(lhsCType)
                        ? apply_integer_promotions(lhsCType) : lhsCType;
                    CType * promoted_right = is_integer_type(rhsCType)
                        ? apply_integer_promotions(rhsCType) : rhsCType;

                    CType * result_type = usual_arithmetic_conversion(promoted_left, promoted_right);
                    node->ctype = get_binary_expr_return_type(result_type, node->binary.op);
                    node->binary.common_type = result_type;
                }
            }
            break;

        case AST_UNARY_EXPR:
            analyze(ctx, node->unary.operand);
            if (node->unary.op == UNARY_DEREF) {
                if (node->unary.operand->ctype->kind == CTYPE_PTR) {
                    node->ctype = node->unary.operand->ctype->base_type;
                    break;
                }
            }
            else if (node->unary.op == UNARY_ADDRESS) {
                node->ctype = make_pointer_type(node->unary.operand->ctype);
                break;
            }
            node->ctype = node->unary.operand->ctype;
            break;

        case AST_VAR_REF_EXPR: {
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            if (!symbol) { error("Symbol not found"); return; }
            node->symbol = symbol;
            // if (symbol->ctype->kind == CTYPE_ARRAY) {
            //      node->ctype = make_pointer_type(symbol->ctype->base_type);
            // }
            // else {
            //     node->ctype = symbol->ctype;
            // }
            node->ctype = symbol->ctype;

            break;
        }

        case AST_RETURN_STMT: {
            analyze(ctx, node->return_stmt.expr);
//            node->ctype = node->return_stmt.expr->ctype;
            node->ctype = ctx->current_function_return_type;
            if (!ctype_equal_or_compatible(ctx->current_function_return_type,
                node->return_stmt.expr->ctype)) {
                if (is_castable(node->ctype, ctx->current_function_return_type)) {
                    node->return_stmt.expr = create_cast_expr_node(ctx->current_function_return_type, node->return_stmt.expr);
                    break;
                } else {
                    char buf_expected[128];
                    buf_expected[0] = '\0';
                    ctype_to_cdecl(ctx->current_function_return_type, buf_expected, sizeof(buf_expected));
                    char buf_actual[128];
                    buf_actual[0] = '\0';
                    ctype_to_cdecl(node->ctype, buf_actual, sizeof(buf_actual));
                    error("Function return type not compatible: expected %s, got %s",
                        buf_expected, buf_actual);
                }
            }
            break;
        }

        case AST_IF_STMT:
            analyze(ctx, node->if_stmt.cond);
            analyze(ctx, node->if_stmt.then_stmt);
            analyze(ctx, node->if_stmt.else_stmt);
            break;

        case AST_WHILE_STMT:
            analyze(ctx, node->while_stmt.cond);
            analyze(ctx, node->while_stmt.body);
            break;

        case AST_FOR_STMT:
            enter_scope();
            analyze(ctx, node->for_stmt.init_expr);
            analyze(ctx, node->for_stmt.cond_expr);
            analyze(ctx, node->for_stmt.update_expr);
            analyze(ctx, node->for_stmt.body);
            exit_scope();
            break;

        case AST_DO_WHILE_STMT:
            analyze(ctx, node->do_while_stmt.expr);
            analyze(ctx, node->do_while_stmt.body);
            break;

        case AST_LABELED_STMT:
            analyze(ctx, node->labeled_stmt.stmt);
            break;

        case AST_CASE_STMT:
            analyze(ctx, node->case_stmt.constExpression);
            analyze(ctx, node->case_stmt.stmt);
            break;

        case AST_DEFAULT_STMT:
            analyze(ctx, node->default_stmt.stmt);
            break;

        case AST_SWITCH_STMT:
            analyze(ctx, node->switch_stmt.expr);
            analyze(ctx, node->switch_stmt.stmt);
            break;

        case AST_CAST_EXPR
            :
            analyze(ctx, node->cast_expr.expr);
//            node->ctype = node->cast_expr.target_ctype;
            break;

        case AST_ARRAY_ACCESS:
            analyze(ctx, node->array_access.base);
            analyze(ctx, node->array_access.index);

//            CType * base_type = decay_if_array(node->array_access.base->ctype);
            CType * base_type = node->array_access.base->ctype;
            CType * index_type = node->array_access.index->ctype;

            if (!(is_pointer_type(base_type) || is_array_type(base_type))) {
                error("Array base must be a pointer or array");
            }

            if (!is_integer_type(index_type)) {
                error("Array index must be an integer");
            }

            node->ctype = base_type->base_type;
            break;

        case AST_INITIALIZER_LIST: {
            assert(node->initializer_list.element_type);
            for (ASTNode_list_node * n = node->initializer_list.items->head; n; n = n->next) {
                analyze(ctx, n->value);
                // if (n->value->type == AST_INITIALIZER_LIST) {
                //     if (is_array_type(node->ctype)) {
                //         node->var_decl.init_expr->initializer_list.element_type = get_base_type(node->ctype);
                //     } else {
                //         node->var_decl.init_expr->initializer_list.element_type = node->ctype;
                //     }
                //     set_element_type(ctx, element_type);
                //     node->var_decl.init_expr->ctype = node->ctype;
                //     analyze(ctx, n->value);
                // } else {
                //     analyze(ctx, n->value);
                //     if (!ctype_equals(n->value->ctype, node->initializer_list.element_type)) {
                //         n->value = create_cast_expr_node(node->initializer_list.element_type, n->value);
                //     }
                // }
            }
            break;
        }

        case AST_STRING_LITERAL: {
            ASTNode_list_append(getTranslationUnit(ctx)->translation_unit.string_literals, node);
            break;
        }

        case AST_FLOAT_LITERAL: {
            ASTNode_list_append(getTranslationUnit(ctx)->translation_unit.float_literals, node);
            break;
        }

        case AST_DOUBLE_LITERAL: {
            ASTNode_list_append(getTranslationUnit(ctx)->translation_unit.double_literals, node);
            break;
        }


        case AST_EXPRESSION_STMT:
        case AST_ASSERT_EXTENSION_STATEMENT:
        case AST_PRINT_EXTENSION_STATEMENT:
            analyze(ctx, node->expr_stmt.expr);
            node->ctype = node->expr_stmt.expr->ctype;
            break;

        case AST_DECLARATION_STMT: {
            for (ASTNode_list_node * n = node->declaration.init_declarator_list->head; n; n = n->next) {
                analyze(ctx, n->value);
                node->ctype = n->value->ctype;
            }
            break;
        }


        case AST_GOTO_STMT:
        case AST_INT_LITERAL:
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
            // DO NOTHING
            break;

        default:
            error("Unrecognized node type - %s", get_ast_node_name(node));
    }

    // check for null ctypes and error out
    verify_expr(ctx, node);
}
