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



// Register order for integer/pointer args in AMD64
// static const char* ARG_REGS[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
// const int ARG_REG_COUNT=6;

//bool emit_print_int_extension = false;

void emit_header(EmitterContext * ctx) {
    emit_line(ctx, "section .text");
    emit_line(ctx, "global main");
    emit_line(ctx, "");
}

void emit_rodata(EmitterContext * ctx, ASTNode_list * string_literals) {
    emit_line(ctx, "");
    emit_line(ctx, "section .rodata");
    emit_line(ctx, "assert_fail_msg: db \"Assertion failed!\", 10");

    for (ASTNode_list_node * n = string_literals->head; n; n = n->next) {
        ASTNode * str_literal = n->value;
        emit_line(ctx, "%s: db %s, 0", str_literal->string_literal.label, escaped_string(str_literal->string_literal.value));
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



void emit_translation_unit(EmitterContext * ctx, ASTNode * node) {
    emit_data_section_header(ctx);

    for (ASTNode_list_node * n = node->translation_unit.string_literals->head; n; n = n->next) {
        ASTNode * str_literal = n->value;
        char * label = make_label_text("Str", get_label_id(ctx));
        str_literal->string_literal.label = label;
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
    emit_rodata(ctx, node->translation_unit.string_literals);
}


void emit_cast(EmitterContext * ctx, ASTNode * node) {

    emit_expr(ctx, node->cast_expr.expr);     // eval inner expression

    CType * from_type = node->cast_expr.expr->ctype;
    CType * to_type = node->cast_expr.target_type;

    int from_size = from_type->size;
    int to_size = to_type->size;

    bool from_signed = from_type->is_signed;
    bool to_signed = to_type->is_signed;

    if ((from_type->kind == CTYPE_PTR || from_type->kind == CTYPE_ARRAY) &&
        (to_type->kind == CTYPE_PTR || to_type->kind == CTYPE_ARRAY)) {
        // pointer to pointer or array to pointer: no-op in codegen
        return;
    }

    if (from_size == to_size) {
        return;  // NOOP
    }

    // narrowing
    if (from_size > to_size) {
        //emit_line(ctx, "pop rax");
        emit_pop(ctx, "rax");
        switch (to_size) {
            case 1: emit_line(ctx, "movsx eax, al"); break;
            case 2: emit_line(ctx, "movsx eax, ax"); break;
            case 4: emit_line(ctx, "mov eax, eax"); break;
            default:
                error("Unsupported narrowing cast to %d bytes", to_size);
        }
        emit_push(ctx, "rax");
//        emit_line(ctx, "push rax");
        return;
    }

    // widening cast
    if (from_size < to_size) {
//        emit_line(ctx, "pop rax");
        emit_pop(ctx, "rax");

        if (from_signed && to_signed) {
            switch (from_size) {
                case 1: emit_line(ctx, "movsx eax, al"); break;
                case 2: emit_line(ctx, "movsx eax, ax"); break;
                case 4: emit_line(ctx, "movsxd rax, eax"); break;
                default:
                    error("Unsupported sign-extension from %d bytes", from_size);
            }
        } else {
            switch (from_size) {
                case 1: emit_line(ctx, "movzx eax, al"); break;
                case 2: emit_line(ctx, "movzx eax, ax"); break;
                case 4: emit_line(ctx, "mov eax, eax"); break;
                default:
                    error("Unsupported zero-extension from %d bytes", from_size);
            }
        }
        emit_push(ctx, "rax");
//        emit_line(ctx, "push rax");
    }
}

void emit_binary_add(EmitterContext * ctx, ASTNode * node) {
    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX

//    emit_line(ctx, "pop rcx");                      // pop rhs to RCX
//    emit_line(ctx, "pop rax");                      // pop lhs to RAX
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    CType *lhs_type = node->binary.lhs->ctype;
    CType *rhs_type = node->binary.rhs->ctype;

    if (lhs_type->kind == CTYPE_PTR && is_integer_type(rhs_type)) {
        int elem_size = sizeof_type(rhs_type);
        emit_line(ctx, "imul rcx, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && rhs_type->kind == CTYPE_PTR) {
        int elem_size = sizeof_type(lhs_type);
        emit_line(ctx, "imul rax, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && is_integer_type(rhs_type)) {
        emit_line(ctx, "; lhs in rax, rhs in rcx");
        if (lhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rax, al");
        } else if (lhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rax, ax");
        } else if (lhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rax, eax");
        }

        if (rhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rcx, cl");
        } else if (rhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rcx, cx");
        } else if (rhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rcx, ecx");
        }

        emit_line(ctx, "add rax, rcx");
    }
    else {
        error("Unsupported types for binary add operation");
    }
//    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    emit_push(ctx, "rax");
//    emit_line(ctx, "push rax");

}

void emit_binary_sub(EmitterContext * ctx, ASTNode * node) {
    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX

    // emit_line(ctx, "pop rcx");                      // pop rhs to RCX
    // emit_line(ctx, "pop rax");                      // pop lhs to RAX
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    CType *lhs_type = node->binary.lhs->ctype;
    CType *rhs_type = node->binary.rhs->ctype;

    if (lhs_type->kind == CTYPE_PTR && is_integer_type(rhs_type)) {
        int elem_size = sizeof_type(lhs_type);
        emit_line(ctx, "imul rcx, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && rhs_type->kind == CTYPE_PTR) {
        int elem_size = sizeof_type(rhs_type);
        emit_line(ctx, "imul rax, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && is_integer_type(rhs_type)) {
        emit_line(ctx, "; lhs in rax, rhs in rcx");
        if (lhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rax, al");
        } else if (lhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rax, ax");
        } else if (lhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rax, eax");
        }

        if (rhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rcx, cl");
        } else if (rhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rcx, cx");
        } else if (rhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rcx, ecx");
        }

        emit_line(ctx, "sub rax, rcx");
    }
    else {
        error("Unsupported types for binary add operation");
    }
    //    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    emit_push(ctx, "rax");
//    emit_line(ctx, "push rax");

}


void emit_binary_multi(EmitterContext * ctx, ASTNode * node) {
    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    // emit_line(ctx, "pop rcx");                      // pop lhs to ECX
    // emit_line(ctx, "pop rax");
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");
    emit_binary_op(ctx, node->binary.op);        // emit proper for op
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");
}

// emit_expr.
// generate code to eval the expression storing the final result in eax or rax
void emit_expr(EmitterContext * ctx, ASTNode * node) {
    switch (node->type) {
        case AST_INT_LITERAL:
            emit_line(ctx, "mov eax, %d", node->int_value);
            //emit_line(ctx, "push rax");
            emit_push(ctx, "rax");
            break;
        case AST_VAR_REF_EXPR: {
            if (node->symbol->ctype->kind == CTYPE_ARRAY) {
                emit_line(ctx, "lea rax, [rbp-%d]", abs(node->symbol->info.var.offset));
//                emit_line(ctx, "push rax");
                emit_push(ctx, "rax");

            }
            else if (node->symbol->node->var_decl.is_global) {
                emit_line(ctx, "mov %s, [rel %s] ", reg_for_type(node->ctype), node->symbol->name);
//                emit_line(ctx, "push rax");
                emit_push(ctx, "rax");

            } else {
                int offset = node->symbol->info.var.offset;
                emit_line(ctx, "mov %s, [rbp%+d]", reg_for_type(node->ctype), offset);
//                emit_line(ctx, "push rax");
                emit_push(ctx, "rax");

            }
            break;
        }
        case AST_UNARY_EXPR:
            emit_unary(ctx, node);
            break;
        case AST_BINARY_EXPR:
            emit_binary_expr(ctx, node);
            break;
        case AST_FUNCTION_CALL_EXPR:
            emit_function_call(ctx, node);
            break;
        case AST_ARRAY_ACCESS:
            emit_addr(ctx, node);
            // add base and index
            if (node->ctype->kind == CTYPE_ARRAY) {
                break;
            }
            emit_pop(ctx, "rcx");


            emit_load_from(ctx, node->ctype, "rcx");                    // load
//            emit_line(ctx, "push rax");
            emit_push(ctx, "rax");


            // emit_line(ctx, "pop rcx");
            // emit_line(ctx, "mov eax, [rcx]");
            // emit_line(ctx, "push rax");

            // emit_expr(ctx, node->array_access.base);                        // emit base
            // emit_expr(ctx, node->array_access.index);                       // emit index
            // emit_line(ctx, "pop rcx");                                  // pop index
            // emit_line(ctx, "pop rax");                                  // pop base
            // emit_line(ctx, "imul rcx, %d", node->ctype->size);          // scale index
            // emit_line(ctx, "add rax, rcx");                             // add base and index
            // if (node->ctype->kind == CTYPE_ARRAY) {
            //     break;
            // }
//            emit_load_from(ctx, node->ctype, "rax");                    // load
//            emit_line(ctx, "movzx eax, %s [rax]", mem_size_for_type(node->ctype));
//            emit_line(ctx, "push rax");
            break;
        case AST_CAST_EXPR:
            // emit_expr(ctx, node->cast_expr.expr);     // eval inner expression
            // emit_line(ctx, "pop rax");
            emit_cast(ctx, node);
            // if (node->ctype->kind == CTYPE_INT) {
            //     emit_line(ctx, "; cast to int: value already in eax");
            // } else if (node->ctype->kind == CTYPE_CHAR) {
            //     emit_line(ctx, "movsx eax, al        ;cast to char");
            // } else if (node->ctype->kind == CTYPE_LONG) {
            //     emit_line(ctx, "movsx rax, eax       ;cast to long");
            // } else if (node->ctype->kind == CTYPE_SHORT) {
            //     emit_line(ctx, "movsx eax, ax        ; cast to short");
            // } else {
            //     error("Unsupported cast type");
            // }
            //emit_line(ctx, "push rax");
            break;
        case AST_STRING_LITERAL: {
            char * label = node->string_literal.label;
            emit_line(ctx, "lea rax, [%s]", label);
//            emit_line(ctx, "push rax");
            emit_push(ctx, "rax");

            break;
        }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}


void emit_addr(EmitterContext * ctx, ASTNode * node) {
    switch (node->type) {
        case AST_VAR_DECL:
        case AST_VAR_REF_EXPR: {

            char * label = create_variable_reference(ctx, node);
            emit_line(ctx, "lea rcx, %s", label);
//            emit_line(ctx, "push rcx");
            emit_push(ctx, "rcx");

            free(label);

            break;
        }
        case AST_UNARY_EXPR:
            if (node->unary.op == UNARY_DEREF) {
                emit_expr(ctx, node->unary.operand);
                emit_line(ctx, "mov rcx, rax");
            }
            else {
                error("Unsupported unary operator in emit_addr");
            }
            break;
        case AST_ARRAY_ACCESS: {

            emit_expr(ctx, node->array_access.base);
//            emit_expr(ctx, node->array_access.index);

            // char * label = create_variable_reference(ctx, node);
            // emit_line(ctx, "lea rcx, %s", label);
            // emit_line(ctx, "push rcx");

//            Symbol * sym = node->array_access.base->symbol;

            CType * base = node->array_access.base->ctype;

//            if (sym->ctype->base_type->kind == CTYPE_PTR) {
            if (base->kind == CTYPE_PTR) {
//                emit_line(ctx, "pop rcx");
                emit_pop(ctx, "rcx");

                emit_line(ctx, "mov rcx, [rcx]");
//                emit_line(ctx, "push rcx");
                emit_push(ctx, "rcx");

            }
            else if (base->kind == CTYPE_ARRAY) {
                // just use base address
            }

            // int base_offset = get_offset(ctx, node);
            // base_offset = abs(base_offset);

            emit_expr(ctx, node->array_access.index);    // put result in eax
            int dim = node->array_access.base->ctype->array_len;
            int base_size = node->ctype->size;
//            emit_line(ctx, "pop rax");

            // if a second dimension.. TODO make work for more than 2.
            if (node->array_access.base->type == AST_ARRAY_ACCESS) {
//                int remaining_size = get_total_nested_array_elements(node->array_access.base);
//                emit_line(ctx, "imul rax, %d", remaining_size);  // scale index
//                emit_line(ctx, "push rax");

                node = node->array_access.base;
                emit_expr(ctx, node->array_access.index);
                //emit_line(ctx, "pop rax");
                emit_pop(ctx, "rax");

                emit_line(ctx, "imul rax, %d", dim);
//                emit_line(ctx, "pop rcx");
                emit_pop(ctx, "rcx");

                emit_line(ctx, "add rax, rcx");
//                emit_line(ctx, "push rax");
                emit_push(ctx, "rax");

            }
//            emit_line(ctx, "imul rax, %d", get_array_base_element_size(node) );  // scale index
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            emit_line(ctx, "imul rax, %d", base_size);

//            emit_line(ctx, "mov rcx, rbp");
//            emit_line(ctx, "sub rcx, %d", base_offset);
//            emit_line(ctx, "pop rcx");
//            emit_line(ctx, "add rcx, rax");
//            emit_line(ctx, "pop rcx");            // pop array base and add offset
            emit_pop(ctx, "rcx");

            emit_line(ctx, "add rcx, rax");

//            emit_line(ctx, "push rcx");
            emit_push(ctx, "rcx");


            // ASTNode * indices[MAX_DIMENSIONS];
            // int index_count = 0;
            // Symbol *base_symbol = NULL;
            // ASTNode * current = node;
            //
            // while (current->type == AST_ARRAY_ACCESS) {
            //     if (index_count >= MAX_DIMENSIONS) {
            //         error("Too many array accesses\n");
            //     }
            //
            //     indices[index_count++] = current->array_access.index;
            //     current = current->array_access.base;
            // }
            //
            // base_symbol = current->symbol;
            //
            // for (int i=0;i<index_count;i++) {
            //     emit_expr(ctx, indices[i]);
            //     if (i == 0) {
            //         emit_line(ctx, "mov rbx, rax\n");
            //     }
            //     else {
            //         int multiplier = 1;
            //         for (int j=i;j<index_count;j++) {
            //             multiplier *= base_symbol->info.array.dimensions[j];
            //         }
            //         emit_line(ctx, "imul rax, rax, %d\n", multiplier);
            //         emit_line(ctx, "add rbx, rax\n");
            //     }
            // }
            //
            // int elem_size = 4;  // int
            // emit_line(ctx, "imul rbx, rbx, %d\n", elem_size);
            // // ASTNode * base = node->array_access.base;
            // // Symbol * symbol = base->symbol;
            // if (is_global_var(ctx, node)) {
            //     emit_line(ctx, "lea rcx [rel %s]\n", base_symbol->name);
            // }
            // else {
            //     int offset = get_offset(ctx, node->array_access.base);
            //     emit_line(ctx, "lea rcx , [rbp%+d]\n", offset);
            // }
            //
            // emit_line(ctx, "add rcx, rbx\n");
            // // emit_expr(ctx, node->array_access.index);
            // //
            // // int elem_size = 4;
            // // emit_line(ctx, "imul rax, rax, %d\n", elem_size);
            // // emit_line(ctx, "add rcx, rax\n");
            break;
        }
        default:
            error("Unexpected node type %s", get_ast_node_name(node));

    }
}

void emit_binary_expr(EmitterContext * ctx, ASTNode *node) {
    switch (node->binary.op) {
        case BINOP_EQ:
        case BINOP_NE:
        case BINOP_GT:
        case BINOP_GE:
        case BINOP_LT:
        case BINOP_LE:
            emit_binary_comparison(ctx, node);
            break;
        case BINOP_ADD:
            emit_binary_add(ctx, node);
            break;
        case BINOP_SUB:
            emit_binary_sub(ctx, node);
            break;
        case BINOP_MUL:
            emit_binary_multi(ctx, node);
   //          emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
   // //         emit_line(ctx, "push rax");                     // push lhs result
   //          emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
   //          emit_line(ctx, "pop rcx");                      // pop lhs to ECX
   //          emit_line(ctx, "pop rax");
   //          emit_binary_op(ctx, node->binary.op);        // emit proper for op
            break;
        case BINOP_DIV:
            emit_binary_div(ctx, node);
            break;
        case BINOP_MOD:
            emit_binary_mod(ctx, node);
            break;
        case BINOP_LOGICAL_AND:
            emit_logical_and(ctx, node);
            break;

        case BINOP_LOGICAL_OR:
            emit_logical_or(ctx, node);
            break;
        case BINOP_ASSIGNMENT:
            emit_assignment(ctx, node);
            break;
        case BINOP_COMPOUND_ADD_ASSIGN:
            emit_add_assignment(ctx, node);
            break;
        case BINOP_COMPOUND_SUB_ASSIGN:
            emit_sub_assignment(ctx, node);
            break;
        default:
            error("Unknown binary operator");
    }
}

void emit_logical_and(EmitterContext * ctx, ASTNode * node) {
    int label_false = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    //lhs 
    emit_expr(ctx, node->binary.lhs);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    //rhs
    emit_expr(ctx, node->binary.rhs);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    // both true
    emit_line(ctx, "mov eax, 1");
    emit_jump(ctx, "jmp", "end", label_end);

    emit_label(ctx, "false", label_false);
    emit_line(ctx, "mov eax, 0");

    emit_push(ctx, "rax");

    emit_label(ctx, "end", label_end);
}

void emit_logical_or(EmitterContext * ctx, ASTNode* node) {
    int label_true = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    // lhs
    emit_expr(ctx, node->binary.lhs);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    // rhs
    emit_expr(ctx, node->binary.rhs);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    emit_label(ctx, "true", label_true);
    emit_line(ctx, "mov eax, 1");
    emit_push(ctx, "rax");

    emit_label(ctx, "end", label_end);
}

void emit_binary_div(EmitterContext * ctx, ASTNode * node) {
    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    // emit_line(ctx, "pop rcx");           // pop rhs to rCX
    // emit_line(ctx, "pop rax");           // pop lhs to rax
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    // emit_line(ctx, "mov ecx, eax");                 // move denominator to ecx
    // emit_line(ctx, "pop rax");                      // restore numerator to eax
    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");


}

void emit_binary_mod(EmitterContext * ctx, ASTNode * node) {

    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    // emit_line(ctx, "pop rcx");           // pop rhs to rCX
    // emit_line(ctx, "pop rax");           // pop lhs to rax
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");               // divide eax by ecx. result goes to eax, remainder to edx
    emit_line(ctx, "mov eax, edx");           // move remainer in edx to eax

//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");

}

void emit_binary_comparison(EmitterContext * ctx, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_expr(ctx, node->binary.lhs);
    emit_expr(ctx, node->binary.rhs);

    // emit_line(ctx, "pop rcx");
    // emit_line(ctx, "pop rax");
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    // eval right-hand side -> reult in eax


    // restore lhs into rcx
    emit_line(ctx, "mov ecx, ecx");   // zero upper bits
    emit_line(ctx, "mov eax, eax");   // zero upper bits

    // compare rax (lhs) with ecx (rhs), cmp rcx, eax means rcx - eax
    emit_line(ctx, "cmp eax, ecx");

    // emit proper setX based on operator type
    switch (node->binary.op) {
        case BINOP_EQ:
            emit_line(ctx, "sete al");
            break;

        case BINOP_NE:
            emit_line(ctx, "setne al");
            break;

        case BINOP_LT:
            emit_line(ctx, "setl al");
            break;

        case BINOP_LE:
            emit_line(ctx, "setle al");
            break;

        case BINOP_GT:
            emit_line(ctx, "setg al");
            break;

        case BINOP_GE:
            emit_line(ctx, "setge al");
            break;

        default:
            error("Unsupported comparison type in codegen.");
    }

    // zero-extend result to full eax
    emit_line(ctx, "movzx eax, al");
    emit_push(ctx, "rax");

}


void emit_binary_op(EmitterContext * ctx, BinaryOperator op) {
    switch(op) {
        case BINOP_ADD:
            emit_line(ctx, "add eax, ecx");
            break;
        case BINOP_SUB:
              emit_line(ctx, "sub ecx, eax");
              emit_line(ctx, "mov eax, ecx");
            break;
        case BINOP_MUL:
            emit_line(ctx, "imul eax, ecx");
            break;
        default:
            error("Unsupported binary operator: %s", token_type_name(op));
    }
}

void emit_unary(EmitterContext * ctx, ASTNode * node) {
    switch (node->unary.op) {
        case UNARY_NEGATE:
            emit_expr(ctx, node->unary.operand);
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            emit_line(ctx, "neg eax");
//            emit_line(ctx, "push rax");
            emit_push(ctx, "rax");

            break;
        case UNARY_PLUS:
            // noop
            break;
        case UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_expr(ctx, node->unary.operand);
            emit_line(ctx, "cmp eax, 0");
            emit_line(ctx, "sete al");
            emit_line(ctx, "movzx eax, al");
            break;
        case UNARY_PRE_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "add eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            free(reference_label);
            break;
        }
        case UNARY_PRE_DEC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "sub eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            free(reference_label);
        break;
        }
        case UNARY_POST_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "mov ecx, eax");
            emit_line(ctx, "add eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            emit_line(ctx, "mov eax, ecx");
            free(reference_label);
            break;
        }
        case UNARY_POST_DEC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "mov ecx, eax");
            emit_line(ctx, "sub eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            emit_line(ctx, "mov eax, ecx");
            free(reference_label);
            break;
        }
        case UNARY_ADDRESS: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "lea rax, %s", reference_label);
//            emit_line(ctx, "push rax");
            emit_push(ctx, "rax");

            free(reference_label);
            break;
        }
        case UNARY_DEREF: {
            emit_expr(ctx, node->unary.operand);
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            if (node->ctype->kind == CTYPE_CHAR) {
                emit_line(ctx, "movzx eax, BYTE [rax]");
            } else if (node->ctype->kind == CTYPE_SHORT) {
                emit_line(ctx, "movzx eax, WORD [rax]");
            } else if (node->ctype->kind == CTYPE_INT) {
                emit_line(ctx, "mov eax, [rax]");
            }
            else if (node->ctype->kind == CTYPE_LONG) {
                emit_line(ctx, "mov rax, [rax]");
            }
//            emit_line(ctx, "push rax");
            emit_push(ctx, "rax");

            break;
        }
        default:
            error("Unsupported unary op in emitter");
    }
}



void emit_if_statement(EmitterContext * ctx, ASTNode * node) {
    int id = get_label_id(ctx);
    // eval condition
    emit_expr(ctx, node->if_stmt.cond);
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
    emit_expr(ctx, node->while_stmt.cond);
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
    emit_expr(ctx, node->do_while_stmt.expr);
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

void emit_function(EmitterContext * ctx, ASTNode * node) {

    // skip forward declarations (only codegen for function definitions that include the body)
    if (node->function_decl.body == NULL) {
        return;
    }
    int prev_stack_depth = ctx->stack_depth;
    ctx->stack_depth = 0;

    char * func_end_label = make_label_text("func_end", get_label_id(ctx));
    push_function_exit_context(ctx, func_end_label);

//    emit_text_section_header(ctx);
    int local_space = node->function_decl.size;

    emit_line(ctx, "%s:", node->function_decl.name);
    //emit_line(ctx, "push rbp           ; creating stack frame");
    emit_push(ctx, "rbp");
    emit_line(ctx,  "mov rbp, rsp");

    if (local_space > 0) {
        int aligned_space = (local_space + 15) & ~15;
        ctx->local_space = aligned_space;
        emit_line(ctx, "sub rsp, %d        ; allocating space for locals", aligned_space);
    }

    if (node->function_decl.param_list) {
        for (const ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
            emit_var_declaration(ctx, n->value);
        }
    }

    emit_block(ctx, node->function_decl.body, false);

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
                emit_expr(ctx, init_value);
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
        emit_expr(ctx, node->var_decl.init_expr);
        emit_addr(ctx, node);
        // emit_line(ctx, "pop rcx");
        // emit_line(ctx, "pop rax");
        emit_pop(ctx, "rcx");
        emit_pop(ctx, "rax");

        emit_line(ctx, "mov %s [rcx], %s",
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

void emit_assignment(EmitterContext * ctx, ASTNode* node) {
    emit_line(ctx, "; emitting assignment - LHS %s = RHS %s",
        get_ast_node_name(node->binary.lhs), get_ast_node_name(node->binary.rhs));
    // eval RHS -> rax then push
    emit_expr(ctx, node->binary.rhs);
//    emit_line(ctx, "push rax");

    // eval LHS addr -> rcx
    emit_addr(ctx, node->binary.lhs);

    // pop LHS into rcx
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // pop RHS in rax
//    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");

    CType * ctype = node->binary.lhs->ctype;

    emit_line(ctx, "mov %s [rcx], %s",
        mem_size_for_type(ctype),
        reg_for_type(ctype));

}

void emit_add_assignment(EmitterContext * ctx, ASTNode * node) {

    // compute lhs address -> rcx
    emit_addr(ctx, node->binary.lhs);

    // load lhs into rax
    emit_line(ctx, "mov DWORD eax, [rcx]");
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");


    // evaluate RHS into eax
    emit_expr(ctx, node->binary.rhs);

    // restore RHS into ecx
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // restore LHS into eax
//    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");


    // add RHS to LHS
    emit_line(ctx, "add eax, ecx");

    // restore LHS address
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");


    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

}

void emit_sub_assignment(EmitterContext * ctx, ASTNode * node) {

    // compute lhs address -> rcx
    emit_addr(ctx, node->binary.lhs);

    // push LHS address on stack
    emit_line(ctx, "mov DWORD eax, [rcx]");
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");


    // // load current value into eax
    // emit_line(ctx, "mov eax, [rcx]");
    //
    // // save current RHS value in eax on stack
    // emit_line(ctx, "push rax");

    // evaluate RHS into eax
    emit_expr(ctx, node->binary.rhs);

//    emit_line(ctx, "mov ecx, eax");

    // restore RHS into ecx
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // restore LHS into eax
//    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");

    // sub RHS from LHS
    emit_line(ctx, "sub eax, ecx");

    // restore LHS address
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");


    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");


    // char * reference_label  = create_variable_reference(ctx, node->binary.lhs);
    // emit_tree_node(ctx, node->binary.rhs);
    // emit_line(ctx, "mov ecx, eax\n");
    // emit_line(ctx, "mov eax, %s\n", reference_label);
    // emit_line(ctx, "sub eax, ecx\n");
    // emit_line(ctx, "mov %s, eax\n", reference_label);
    // free(reference_label);
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
        emit_expr(ctx, node->for_stmt.update_expr);
    }

    // loop condition
    emit_label_from_text(ctx, condition_label);
    if (node->for_stmt.cond_expr) {
        emit_expr(ctx, node->for_stmt.cond_expr);
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

void emit_function_call(EmitterContext * ctx, ASTNode * node) {
    // if the call has arguments
    // first get a reversed list
    // then emit each arg then push it
    //struct node_list * reversed_list = NULL;

    int arg_count=0;
    if (node->function_call.arg_list) {

        // loop through in reverse order pushing arguments to the stack
        for (int i = node->function_call.arg_list->count - 1; i >= 0; i--) {
            ASTNode * argNode = ASTNode_list_get(node->function_call.arg_list, i);
            emit_expr(ctx, argNode);
//            emit_line(ctx, "push rax");
            arg_count++;
        }

        // for (ASTNode_list_node *n = node->function_call.arg_list->head;n;n=n->next) {
        //     ASTNode * argNode = n->value;
        //     emit_tree_node(ctx, argNode);
        //     emit_line(ctx, "push rax\n");
        //     // if (arg_count<ARG_REG_COUNT) {
        //     //     const char * reg = ARG_REGS[arg_count];
        //     //     emit_line(ctx, "mov %s, rax\n", reg);
        //     // }  //TODO SUPPORT MORE THAN 6 arguments using the stack
        // }

    }

    // // call the function
    emit_line(ctx, "call %s", node->function_call.name);

    // // clean up arguments
    if (arg_count > 0) {
        emit_add_rsp(ctx, arg_count*8);
    }

//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");

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

    emit_expr(ctx, node->switch_stmt.expr);
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

void emit_array_access(EmitterContext * ctx, ASTNode * node) {
//    char * var_ref = create_variable_reference(ctx, node);

    int base_address = get_offset(ctx, node);

    // generate code for index
    emit_expr(ctx, node->array_access.index);    // result in eax

    emit_line(ctx, "imul eax, eax, %d", node->ctype->size);    // scale index
    emit_line(ctx, "mov rdx, rax");
    emit_line(ctx, "DWORD PTR [rbp-%d]", base_address);
}

void emit_tree_node(EmitterContext * ctx, ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_TRANSLATION_UNIT:
            emit_translation_unit(ctx, node);
            break;
        case AST_FUNCTION_DECL:
            emit_function(ctx, node);
            break;
        case AST_VAR_DECL:
            emit_var_declaration(ctx, node);
            break;
        case AST_RETURN_STMT:
            emit_expr(ctx, node->return_stmt.expr);
            if (ctx->functionExitStack && ctx->functionExitStack->exit_label) {
                emit_jump_from_text(ctx, "jmp", ctx->functionExitStack->exit_label);
            }
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            break;
        case AST_FUNCTION_CALL_EXPR:
            emit_expr(ctx, node);
            //            emit_function_call(ctx, node);
            break;
        case AST_EXPRESSION_STMT:
            //            emit_tree_node(ctx, node->expr_stmt.expr);
            emit_expr(ctx, node->expr_stmt.expr);
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
        case AST_VAR_REF_EXPR: {
            emit_expr(ctx, node);
            break;
        }
        case AST_ARRAY_ACCESS: {
            emit_expr(ctx, node);
            // int offset = get_offset(ctx, node);
            // emit_line(ctx, "mov eax, [rbp%+d]", offset);
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
