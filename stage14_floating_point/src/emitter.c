#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "list_util.h"
#include "c_type.h"
#include "ast.h"
#include "emitter.h"

#include "token.h"
#include "util.h"
#include "error.h"
#include "emitter_context.h"
#include "emit_extensions.h"
#include "symbol.h"
#include "emitter_helpers.h"
#include "emit_address.h"
#include "emit_expression.h"


// Register order for integer/pointer args in AMD64
// static const char* ARG_REGS[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
// const int ARG_REG_COUNT=6;

//bool emit_print_int_extension = false;

void emit_header(EmitterContext * ctx) {
    emit_line(ctx, "section .text");
    emit_line(ctx, "global main");
    emit_line(ctx, "");
}

void emit_rodata(EmitterContext * ctx, ASTNode_list * string_literals, ASTNode_list * float_literals, ASTNode_list * double_literals) {
    emit_line(ctx, "");
    emit_line(ctx, "section .rodata");
    emit_line(ctx, "assert_fail_msg: db \"Assertion failed!\", 10");

    for (ASTNode_list_node * n = string_literals->head; n; n = n->next) {
        ASTNode * str_literal = n->value;
        emit_string_literal(ctx, str_literal->string_literal.label, str_literal->string_literal.value);
//        emit_line(ctx, "%s: db %s, 0", str_literal->string_literal.label, escaped_string(str_literal->string_literal.value));
    }

    for (ASTNode_list_node * n = float_literals->head; n; n = n->next) {
        ASTNode * float_literal = n->value;
        emit_float_literal(ctx, float_literal->float_literal.label, float_literal->float_literal.value);
        //        emit_line(ctx, "%s: db %s, 0", str_literal->string_literal.label, escaped_string(str_literal->string_literal.value));
    }

    for (ASTNode_list_node * n = double_literals->head; n; n = n->next) {
        ASTNode * double_literal = n->value;
        emit_double_literal(ctx, double_literal->double_literal.label, double_literal->double_literal.value);
        //        emit_line(ctx, "%s: db %s, 0", str_literal->string_literal.label, escaped_string(str_literal->string_literal.value));
    }

}

void emit_text_section_header(EmitterContext * ctx) {
    emit_line(ctx, "");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, ";   SECTION: Text (Code)");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, "");
    emit_line(ctx, "section .text");
    emit_line(ctx, "global main");
    emit_line(ctx, "");
}

void emit_data_section_header(EmitterContext * ctx) {
    emit_line(ctx, "");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, ";   SECTION: Data (Initialized globals/strings");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, "");
    emit_line(ctx, "section .data");
    emit_line(ctx, "");
}

void emit_bss_section_header(EmitterContext * ctx) {
    emit_line(ctx, "");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, ";   SECTION: BSS (Uninitialized buffers)");
    emit_line(ctx, ";---------------------------------------");
    emit_line(ctx, "");
    emit_line(ctx, "section .bss");
    emit_line(ctx, "");
}

void emit_jump(EmitterContext * ctx, const char * op, const char * prefix, int num) {
    emit_line(ctx, "%s L%s%d", op, prefix, num);
}

void emit_jump_from_text(EmitterContext * ctx, const char * op, const char * label) {
    emit_line(ctx, "%s L%s", op, label);
}

char * get_string_literal_label(ASTNode* node, char * literal) {
    for (ASTNode_list_node * n = node->translation_unit.string_literals->head; n; n = n->next) {
        ASTNode * str_literal = n->value;
        if (strcmp(str_literal->string_literal.value, literal) == 0) {
            return str_literal->string_literal.label;
        }
    }
    return "\0";
}

void emit_translation_unit(EmitterContext * ctx, ASTNode * node) {
    emit_data_section_header(ctx);

    for (ASTNode_list_node * n = node->translation_unit.string_literals->head; n; n = n->next) {
        ASTNode * str_literal = n->value;
        char * label = make_label_text("Str", get_label_id(ctx));
        str_literal->string_literal.label = label;
    }

    for (ASTNode_list_node * n = node->translation_unit.float_literals->head; n; n = n->next) {
        ASTNode * float_literal = n->value;
        char * label = make_label_text("Flt", get_label_id(ctx));
        float_literal->float_literal.label = label;
    }

    for (ASTNode_list_node * n = node->translation_unit.double_literals->head; n; n = n->next) {
        ASTNode * double_literal = n->value;
        char * label = make_label_text("Dbl", get_label_id(ctx));
        double_literal->float_literal.label = label;
    }

    for (ASTNode_list_node * n = node->translation_unit.globals->head; n; n = n->next) {
        ASTNode * global_var = n->value;
        if (global_var->var_decl.init_expr) {
            // TODO write out correct emit_tree_node(ctx, n->value);
            char * data_directive = get_data_directive(global_var->ctype);
            if (is_array_type(global_var->ctype)) {
                ASTNode * init_expr = global_var->var_decl.init_expr;
                ASTNode_list * init_list = init_expr->initializer_list.items;
                ASTNode_list * flattened_list = create_node_list();
                flatten_list(init_list, flattened_list);
                char buff[1024];
                buff[0] = '\0';
                int total_elements = get_total_nested_array_elements(global_var);
                if (total_elements < flattened_list->count) {
                    error("Array has too many items in initializer");
                }
                size_t used = snprintf(buff, sizeof(buff), "%s:   %s ", global_var->var_decl.name, data_directive);
                for (int i = 0; i < total_elements; i++) {
                    if (i > 0) {
                        used += snprintf(buff + used, sizeof(buff) - used, ",");
                    }
                    if (i < flattened_list->count) {
                        int value = ASTNode_list_get(flattened_list, i)->int_value;
                        used += snprintf(buff+used, sizeof(buff) - used, " %d", value);
                    }
                    else {
                        used += snprintf(buff+used, sizeof(buff)  -used,  " 0");
                    }
                }
                emit_line(ctx, buff);
                // global_var->ctype->array_len < global_var->var_decl.init_expr ? 1 : 0;
                // for (int i=0;global_var->)
            }
            else if (is_pointer_type(global_var->ctype) && global_var->ctype->base_type->kind == CTYPE_CHAR ) {
                char * literal_label = get_string_literal_label(node, global_var->var_decl.init_expr->string_literal.value);
                emit_line(ctx, "%s: %s %s", global_var->var_decl.name, data_directive, literal_label);
            }
            else {
                emit_line(ctx, "%s: %s %d", global_var->var_decl.name, data_directive, global_var->var_decl.init_expr->int_value);
            }
        }
    }

    emit_bss_section_header(ctx);
    for (ASTNode_list_node * n = node->translation_unit.globals->head; n; n = n->next) {
        ASTNode * global_var = n->value;
        if (!global_var->var_decl.init_expr) {
            // TODO write out correct emit_tree_node(ctx, n->value);
            char * reservation_directive = get_reservation_directive(global_var->ctype);
            int size = (is_array_type(global_var->ctype) ? global_var->ctype->array_len : 1);
            emit_line(ctx, "%s: %s %d", global_var->var_decl.name, reservation_directive, size);
        }
    }

    emit_text_section_header(ctx);
    for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
        emit_tree_node(ctx, n->value);
    }
    emit_rodata(ctx, node->translation_unit.string_literals, node->translation_unit.float_literals, node->translation_unit.double_literals);
}

void emit_if_statement(EmitterContext * ctx, ASTNode * node) {
    int id = get_label_id(ctx);
    // eval condition
    emit_int_expr_to_rax(ctx, node->if_stmt.cond, WANT_VALUE);
    emit_pop(ctx, "rax");
    // compare result with 0
    emit_line(ctx, "cmp eax, 0");

    if (node->if_stmt.else_stmt) {
        emit_line(ctx, "je Lelse%d", id);  // jump to else if false
        emit_tree_node(ctx, node->if_stmt.then_stmt);
        emit_line(ctx, "jmp Lend%d", id);  // jump to end over else
        emit_label(ctx, "else", id);
        emit_tree_node(ctx, node->if_stmt.else_stmt);
    }
    else {
        emit_line(ctx, "je Lend%d", id);  // skip over if false
        emit_tree_node(ctx, node->if_stmt.then_stmt);
    }
    emit_label(ctx, "end", id);
}

void emit_while_statement(EmitterContext * ctx, ASTNode * node) {

    char * loop_start_label = make_label_text("while_start", get_label_id(ctx));
    char * loop_end_label = make_label_text("while_end", get_label_id(ctx));

    //emit_expr(ctx, node->switch_stmt.expr);
    //emit_line(ctx, "push rax   ; save switch expression");
    //emit_push(ctx, "rax");


    push_loop_context(ctx, loop_start_label, loop_end_label);

    // start label
    emit_label_from_text(ctx, loop_start_label);
    // eval cond
    emit_int_expr_to_rax(ctx, node->while_stmt.cond, WANT_VALUE);
    emit_pop(ctx, "rax");
    // cmp to zero
    emit_line(ctx, "cmp eax, 0");
    // jmp to end if condition not met
    emit_jump_from_text(ctx, "je", loop_end_label);
    emit_tree_node(ctx, node->while_stmt.body);
    // jmp to start
    emit_jump_from_text(ctx, "jmp", loop_start_label);
    emit_label_from_text(ctx, loop_end_label);

    free(loop_start_label);
    free(loop_end_label);

}

void emit_do_while_statement(EmitterContext * ctx, ASTNode * node) {
    int id = get_label_id(ctx);
    emit_label(ctx, "do_while_start", id);
    emit_line(ctx, "; emitting do_while main body statement");
    emit_tree_node(ctx, node->do_while_stmt.body);

    emit_line(ctx, "; emitting do_while condition expression");
    emit_int_expr_to_rax(ctx, node->do_while_stmt.expr, WANT_VALUE);
    emit_pop(ctx, "rax");

    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "do_while_start", id);
    emit_label(ctx, "do_while_end", id);
}

void emit_block(EmitterContext * ctx, ASTNode * node, bool enterNewScope) {

    int count = 0;
    for (const ASTNode_list_node * n = node->block.statements->head; n; n = n->next) {
        emit_line(ctx,"; emitting function statement # %d of type: %s", ++count, get_ast_node_name(n->value));
        emit_tree_node(ctx, n->value);
    }

}

void emit_function_definition(EmitterContext * ctx, ASTNode * node) {

    // // skip forward declarations (only codegen for function definitions that include the body)
    // if (node->function_decl.body == NULL) {
    //     return;
    // }
    int prev_stack_depth = ctx->stack_depth;
    ctx->stack_depth = 0;

    char * func_end_label = make_label_text("func_end", get_label_id(ctx));
    push_function_exit_context(ctx, func_end_label);

//    emit_text_section_header(ctx);
    int local_space = node->function_def.size;

    emit_line(ctx, "%s:", node->function_def.name);
    //emit_line(ctx, "push rbp           ; creating stack frame");
    emit_push(ctx, "rbp");
    emit_line(ctx,  "mov rbp, rsp");

    if (local_space > 0) {
        int aligned_space = (local_space + 15) & ~15;
        ctx->local_space = aligned_space;
        emit_line(ctx, "sub rsp, %d        ; allocating space for locals", aligned_space);
    }

    if (node->function_def.param_list) {
        for (const ASTNode_list_node * n = node->function_def.param_list->head;n;n=n->next) {
            emit_var_declaration(ctx, n->value);
        }
    }

    emit_block(ctx, node->function_def.body, false);

    emit_label_from_text(ctx, func_end_label);
    emit_leave(ctx);
    emit_line(ctx, "ret");

    assert(ctx->stack_depth == 0);
    ctx->stack_depth = prev_stack_depth;

    pop_function_exit_context(ctx);
    free(func_end_label);
}

void emit_var_declaration(EmitterContext * ctx, ASTNode * node) {
    emit_line(ctx, "; emitting variable '%s' declaration", node->symbol->name);
    if (!node->var_decl.init_expr) {
        emit_line(ctx, "; declaration only (no initialization)");
        return;
    }

    if (node->var_decl.init_expr->type == AST_INITIALIZER_LIST) {
        emit_line(ctx,"; initializing array");
        Symbol * symbol = node->symbol;
        ASTNode_list * init_items = node->var_decl.init_expr->initializer_list.items;
        ASTNode_list * flattened_list = create_node_list();
        flatten_list(init_items, flattened_list);
        int total_items = get_total_nested_array_elements(node);
//            for (int i=0;i<init_items->count;i++) {
//            for (int i=0;i<node->ctype->array_len;i++) {
        for (int i = 0; i < total_items; i++) {
            emit_line(ctx, "; initializing element %d", i);

            if (i < flattened_list->count) {
                ASTNode * init_value = ASTNode_list_get(flattened_list, i);
                emit_int_expr_to_rax(ctx, init_value, WANT_VALUE);
            }
            else {
                emit_line(ctx, "mov eax, 0");
                emit_line(ctx, "push rax");
            }

            int offset = symbol->info.array.offset + i*sizeof_basetype(node->ctype);

            emit_line(ctx, "lea rcx, [rbp%+d]", offset);

//                emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            emit_line(ctx, "mov [rcx], eax");
        }
    }
    else {
        emit_line(ctx,"; initializing variable");
        if (is_array_type(node->var_decl.init_expr->ctype)) {
            emit_addr_to_rax(ctx, node->var_decl.init_expr);
        } else if (is_floating_point_type(node->var_decl.init_expr->ctype)) {
            emit_fp_expr_to_xmm0(ctx, node->var_decl.init_expr, WANT_VALUE);
        }
        else {
            emit_int_expr_to_rax(ctx, node->var_decl.init_expr, WANT_VALUE);
        }
        emit_addr_to_rax(ctx, node);
        // emit_line(ctx, "pop rcx");
        // emit_line(ctx, "pop rax");
        emit_pop(ctx, "rcx");
        //emit_pop(ctx, "rax");
        emit_pop_for_type(ctx, node->ctype);

        // emit a line to copy register value to proper address
        // example: mov [rcx], eax
        emit_line(ctx, "%s %s [rcx], %s",
            mov_instruction_for_type(node->ctype),
            mem_size_for_type(node->ctype),
            reg_for_type(node->ctype));
        // if (node->ctype->kind == CTYPE_CHAR) {
        //     emit_line(ctx, "mov BYTE [rcx], al\n");
        // } else if (node->ctype->kind == CTYPE_SHORT) {
        //     emit_line(ctx, "mov WORD [rcx], ax\n");
        // } else if (node->ctype->kind == CTYPE_INT) {
        //     emit_line(ctx, "mov DWORD [rcx], eax\n");
        // } else if (node->ctype->kind == CTYPE_LONG || node->ctype->kind == CTYPE_PTR) {
        //     emit_line(ctx, "mov QWORD [rcx], rax\n");
        // }
        // else {
        //     error("Unsupported type");
        // }
    }
    // char * reference_label = create_variable_reference(ctx, node);
    // emit_tree_node(ctx, node->var_decl.init_expr);
    // emit_line(ctx, "mov %s, eax\n", reference_label);
}



void emit_for_statement(EmitterContext * ctx, ASTNode * node) {

    char * start_label = make_label_text("for_start", get_label_id(ctx));
    char * end_label = make_label_text("for_end", get_label_id(ctx));
    char * condition_label = make_label_text("for_condition", get_label_id(ctx));
    char * continue_label = make_label_text("for_continue", get_label_id(ctx));

    push_loop_context(ctx, continue_label, end_label);

    // initializer
    if (node->for_stmt.init_expr) {
        // for initializer could be a variable declaration or an expression so use emit_tree_node
        emit_tree_node(ctx, node->for_stmt.init_expr);
    }

    // jump to condition check
    emit_jump_from_text(ctx, "jmp", condition_label);

    // jump label (target) for main body start
    emit_label_from_text(ctx, start_label);

    // loop body start
    emit_block(ctx, node->for_stmt.body, false);

    // emit continue label
    emit_label_from_text(ctx, continue_label);

    // update expression
    if (node->for_stmt.update_expr) {
        emit_int_expr_to_rax(ctx, node->for_stmt.update_expr, WANT_EFFECT);
    }

    // loop condition
    emit_label_from_text(ctx, condition_label);
    if (node->for_stmt.cond_expr) {
        emit_int_expr_to_rax(ctx, node->for_stmt.cond_expr, WANT_VALUE);
        emit_pop(ctx, "rax");
        emit_line(ctx, "cmp eax, 0");
        emit_jump_from_text(ctx, "je", end_label);     // exit if false
    }

    emit_jump_from_text(ctx, "jmp", start_label);
    
    // end/break label
    emit_label_from_text(ctx, end_label);

    pop_loop_context(ctx);

    free(start_label);
    free(end_label);
    free(condition_label);
    free(continue_label);

}

void emit_pass_argument(EmitterContext * ctx, CType * type, ASTNode * node) {
    emit_tree_node(ctx, node);
    char * reference_label = create_variable_reference(ctx, node);
    emit_line(ctx, "mov %s %s, %s",
        mem_size_for_type(type),
        reference_label,
        reg_for_type(type));
    // switch(type->size) {
    //     case 1:
    //         emit_line(ctx, "mov byte %s, al\n", reference_label);
    //         break;
    //     case 2:
    //         emit_line(ctx, "mov word %s, ax\n", reference_label);
    //         break;
    //     case 4:
    //         emit_line(ctx, "mov dword %s, eax\n", reference_label);
    //         break;
    //     case 8:
    //         emit_line(ctx, "mov qword %s, rax\n", reference_label);
    //         break;
    //     default:
    //         error("Unsupported type size %d in emit_pass_argument\n", type->size);
    // }
    free(reference_label);
}


void emit_switch_dispatch(EmitterContext * ctx, ASTNode * node) {
    assert(node->type == AST_SWITCH_STMT);
    ASTNode * block = node->switch_stmt.stmt;
    assert(block->type == AST_BLOCK_STMT);

    for (ASTNode_list_node * n = block->block.statements->head; n; n = n->next) {
        ASTNode * statement = n->value;        
        if (statement->type == AST_CASE_STMT) {
            statement->case_stmt.label = make_label_text("case", get_label_id(ctx));
//            runtime_info(statement)->label = make_label_text("case", get_label_id(ctx));
            emit_line(ctx, "mov rax, [rsp]");
            emit_line(ctx, "cmp rax, %d", statement->case_stmt.constExpression->int_value);
            emit_line(ctx, "je %s",  statement->case_stmt.label);
        }
        else if (statement->type == AST_DEFAULT_STMT) {
            statement->default_stmt.label = make_label_text("default", get_label_id(ctx));
            emit_line(ctx, "jmp %s", statement->default_stmt.label);
        }
    }
}

void emit_switch_bodies(EmitterContext * ctx, ASTNode * node) {
        assert(node->type == AST_SWITCH_STMT);
    ASTNode * block = node->switch_stmt.stmt;
    assert(block->type == AST_BLOCK_STMT);

    for (ASTNode_list_node * n = block->block.statements->head; n; n = n->next) {
        ASTNode * statement = n->value;

        if (statement->type == AST_CASE_STMT) {
            emit_line(ctx, "\n%s:", statement->case_stmt.label);
            emit_tree_node(ctx, statement->case_stmt.stmt);
        }
        else if (statement->type == AST_DEFAULT_STMT) {
            emit_line(ctx, "\n%s:", statement->default_stmt.label);
            emit_tree_node(ctx, statement->default_stmt.stmt);
        }
    }

}

void emit_switch_statement(EmitterContext * ctx, ASTNode * node) {
    assert(node->type == AST_SWITCH_STMT);
    int label_end = get_label_id(ctx);

    char * break_label = make_label_text("switch_end", label_end);

    emit_int_expr_to_rax(ctx, node->switch_stmt.expr, WANT_VALUE);
//    emit_line(ctx, "push rax   ; save switch expression\n");
//    emit_pop(ctx, "rax");

    push_switch_context(ctx, break_label);

    // first pass emit all comparisons and jumps
    emit_switch_dispatch(ctx, node);

    emit_pop(ctx, "rax");

    // second pass emit all labels and bodies
    emit_switch_bodies(ctx, node);

    emit_line(ctx, "\n%s:", break_label);

    pop_switch_context(ctx);

    free(break_label);
}

void emit_case_statement(EmitterContext * ctx, ASTNode * node) {
    int case_label_id = get_label_id(ctx);
    char * case_label = make_label_text("case", case_label_id);
    node->case_stmt.label = strdup(case_label);

    // load switch value back from the stack
    emit_line(ctx, "mov rax, [rsp] ; reload switch expr");
    
    // emit the jmp to the case body
    emit_line(ctx, "cmp rax, %d", node->case_stmt.constExpression->int_value);
    emit_line(ctx, "je %s", case_label);

    // emit the case body
    emit_line(ctx, "%s", node->case_stmt.label);
    emit_tree_node(ctx, node->case_stmt.stmt);
    // emit jump to break

    emit_line(ctx, "jmp %s", current_switch_break_label(ctx));

    free(case_label);
}

const char * get_break_label(EmitterContext * ctx) {
    if (ctx->loop_stack) return ctx->loop_stack->end_label;
    return current_switch_break_label(ctx);
}

const char * get_continue_label(EmitterContext * ctx) {
    if (ctx->loop_stack) return ctx->loop_stack->start_label;
    return NULL;
}

void emit_break_statement(EmitterContext * ctx, ASTNode * node) {
    const char * label = get_break_label(ctx);
    emit_jump_from_text(ctx, "jmp", label);
}

void emit_continue_statement(EmitterContext * ctx, ASTNode * node) {
    const char * label = get_continue_label(ctx);
    emit_jump_from_text(ctx, "jmp", label);
}

// void emit_array_access(EmitterContext * ctx, ASTNode * node) {
// //    char * var_ref = create_variable_reference(ctx, node);
//
//     int base_address = get_offset(ctx, node);
//
//     // generate code for index
//     emit_expr(ctx, node->array_access.index);    // result in eax
//
//     emit_line(ctx, "imul eax, eax, %d", node->ctype->size);    // scale index
//     emit_line(ctx, "mov rdx, rax");
//     emit_line(ctx, "DWORD PTR [rbp-%d]", base_address);
// }

void emit_tree_node(EmitterContext * ctx, ASTNode * node) {
    if (!node) return;

    switch(node->type) {
        case AST_TRANSLATION_UNIT:
            emit_translation_unit(ctx, node);
            break;
        case AST_FUNCTION_DECL:
            //emit_function(ctx, node);
            // noop
            break;
        case AST_FUNCTION_DEF:
            emit_function_definition(ctx, node);
            break;
        case AST_VAR_DECL:
            emit_var_declaration(ctx, node);
            break;
        case AST_RETURN_STMT:
            if (is_integer_type(node->ctype)) {
                emit_int_expr_to_rax(ctx, node->return_stmt.expr, WANT_VALUE);
                if (ctx->functionExitStack && ctx->functionExitStack->exit_label) {
                    emit_jump_from_text(ctx, "jmp", ctx->functionExitStack->exit_label);
                }
                //            emit_line(ctx, "pop rax");
                emit_pop(ctx, "rax");
            }
            else if (is_floating_point_type(node->ctype)) {
                emit_fp_expr_to_xmm0(ctx, node->return_stmt.expr, WANT_VALUE);
                if (ctx->functionExitStack && ctx->functionExitStack->exit_label) {
                    emit_jump_from_text(ctx, "jmp", ctx->functionExitStack->exit_label);
                }
                //            emit_line(ctx, "pop rax");
                emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            else {
                error("invalid return statement type");
            }
            break;
        case AST_FUNCTION_CALL_EXPR:
            emit_int_expr_to_rax(ctx, node, WANT_VALUE);
            //            emit_function_call(ctx, node);
            break;
        case AST_EXPRESSION_STMT:
            //            emit_tree_node(ctx, node->expr_stmt.expr);
            emit_int_expr_to_rax(ctx, node->expr_stmt.expr, WANT_EFFECT);
            break;
        case AST_ASSERT_EXTENSION_STATEMENT:
            emit_assert_extension_statement(ctx, node);
            break;
        case AST_PRINT_EXTENSION_STATEMENT:
            emit_print_int_extension_call(ctx, node);
            break;
        case AST_BLOCK_STMT:
            emit_block(ctx, node, true);
            break;
        case AST_IF_STMT:
            emit_if_statement(ctx, node);
            break;
        case AST_WHILE_STMT:
            emit_while_statement(ctx, node);
            break;
        case AST_DO_WHILE_STMT:
            emit_do_while_statement(ctx, node);
            break;
        case AST_SWITCH_STMT:
            emit_switch_statement(ctx, node);
            break;
        case AST_CASE_STMT:
            emit_case_statement(ctx, node);
            break;
        case AST_BREAK_STMT:
            emit_break_statement(ctx, node);
            break;
        case AST_CONTINUE_STMT:
            emit_continue_statement(ctx, node);
            break;

        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
        case AST_INT_LITERAL:
        case AST_CAST_EXPR:
        case AST_VAR_REF_EXPR:
        case AST_ARRAY_ACCESS:
        case AST_FLOAT_LITERAL:
        case AST_DOUBLE_LITERAL: {
            if (is_floating_point_type(node->ctype)) {
                emit_fp_expr_to_xmm0(ctx, node, WANT_VALUE);
            } else {
                emit_int_expr_to_rax(ctx, node, WANT_VALUE);
            }
            break;
        }
        case AST_INITIALIZER_LIST: {
            for (int i=0; i<node->initializer_list.items->count;i++) {

            }
            break;
        }

        case AST_STRING_LITERAL: {
            //TODO
            break;
        }

        case AST_FOR_STMT:
            emit_for_statement(ctx, node);
            break;
        case AST_GOTO_STMT:
            emit_jump(ctx, "jmp", node->goto_stmt.label, 0);
            break;

        case AST_LABELED_STMT:
            emit_label(ctx, node->labeled_stmt.label, 0);
            emit_tree_node(ctx, node->labeled_stmt.stmt);
            break;

        case AST_DECLARATION_STMT: {
            for (ASTNode_list_node * n = node->declaration.init_declarator_list->head; n; n = n->next) {
                emit_tree_node(ctx, n->value);
            }
            break;
        }
        // case AST_CAST_EXPR:
        //     emit_expr(ctx, node->cast_expr.expr);
        //     emit_cast(ctx, node->cast_expr.expr->ctype, node->cast_expr.target_type);
        //     break;
        default:
            error("Unhandled type %s\n", node->type);
            break;

    }
}


void emit(EmitterContext * ctx, ASTNode * translation_unit) {
   // populate_symbol_table(translation_unit);
    emit_tree_node(ctx, translation_unit);

    // emit referenced private functions
    if (ctx->emit_print_int_extension) {
        emit_print_int_extension_function(ctx);
    }

}
