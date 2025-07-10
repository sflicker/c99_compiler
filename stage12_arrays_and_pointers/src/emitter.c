#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "c_type.h"
#include "ast.h"
#include "emitter.h"
#include "token.h"
#include "util.h"
#include "error.h"
#include "emitter_context.h"
#include "symbol.h"


// Register order for integer/pointer args in AMD64
// static const char* ARG_REGS[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
// const int ARG_REG_COUNT=6;

//bool emit_print_int_extension = false;

char * create_variable_reference(EmitterContext * ctx, ASTNode * node) {
    if (is_global_var(ctx, node)) {
        const char * name = get_var_name(ctx, node);
        int size = snprintf(NULL, 0, "[rel %s]", name) + 1;
        char * label = malloc(size);
        snprintf(label, size, "[rel %s]", name);
        return label;
    }
    else {
        int offset = get_offset(ctx, node);
        int size = snprintf(NULL, 0, "[rbp%+d]", offset) + 1;
        char * label = malloc(size);
        snprintf(label, size, "[rbp%+d]", offset);
        return label;
    }
}

const char * reg_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "al";
        case CTYPE_SHORT: return "ax";
        case CTYPE_INT: return "eax";
        case CTYPE_LONG: return "rax";
        case CTYPE_PTR: return "rax";
        default: error("Unsupported type");
    }
    return NULL;
}

const char * mem_size_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "BYTE";
        case CTYPE_SHORT: return "WORD";
        case CTYPE_INT: return "DWORD";
        case CTYPE_LONG: return "QWORD";
        case CTYPE_PTR:  return "QWORD";
        default:
            error("Unsupported type");
    }
    return NULL;
}

void emit_line(EmitterContext * ctx, const char* fmt, ...) {
    va_list args;

    // --- 1. Write to the file
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
    fputc('\n', ctx->out);

    // --- 2. Echo to stdout (re-initialize args)
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fputc('\n', stdout);
}

void emit_header(EmitterContext * ctx) {
    emit_line(ctx, "section .text");
    emit_line(ctx, "global main");
    emit_line(ctx, "");
}

void emit_trailer(EmitterContext * ctx) {
    emit_line(ctx, "");
    emit_line(ctx, "section .rodata");
    emit_line(ctx, "assert_fail_msg: db \"Assertion failed!\", 10");
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

char * make_label_text(const char * prefix, int num) {
    size_t buffer_size = strlen(prefix) + 20;
    char * label = malloc(buffer_size);
    snprintf(label, buffer_size, "%s%d", prefix, num);
    return label;
}

void emit_label(EmitterContext * ctx, const char * prefix, int num) {
    emit_line(ctx, "L%s%d:", prefix, num);
}

void emit_label_from_text(EmitterContext *ctx, const char * label) {
    emit_line(ctx, "L%s:", label);
}

void emit_jump(EmitterContext * ctx, const char * op, const char * prefix, int num) {
    emit_line(ctx, "%s L%s%d", op, prefix, num);
}

void emit_jump_from_text(EmitterContext * ctx, const char * op, const char * label) {
    emit_line(ctx, "%s L%s", op, label);
}

char * get_data_directive(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "db";
        case CTYPE_SHORT: return "dw";
        case CTYPE_INT: return "dd";
        case CTYPE_LONG: return "dw";
        default:
            error("Unsupported data type");
    }
    return NULL;
}

char * get_reservation_directive(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "resb 1";
        case CTYPE_SHORT: return "resw 1";
        case CTYPE_INT: return "resd 1";
        case CTYPE_LONG: return "resq 1";
        default:
            error("Unsupported data type");
    }
    return NULL;
}

void emit_cast(EmitterContext * ctx, CType * from_type, CType * to_type) {
    int from_size = from_type->size;
    int to_size = to_type->size;

    bool from_signed = from_type->is_signed;
    bool to_signed = to_type->is_signed;

    if (from_size == to_size) {
        return;  // NOOP
    }

    // narrowing
    if (from_size > to_size) {
        switch (to_size) {
            case 1: emit_line(ctx, "movsx eax, al"); break;
            case 2: emit_line(ctx, "movsx eax, ax)"); break;
            case 4: emit_line(ctx, "mov eax, eax"); break;
            default:
                error("Unsupported narrowing cast to %d bytes", to_size);
        }
        return;
    }

    // widening cast
    if (from_size < to_size) {
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
    }
}

void emit_binary_add(EmitterContext * ctx, ASTNode * node) {
    emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX

    emit_line(ctx, "pop rcx");                      // pop rhs to RCX
    emit_line(ctx, "pop rax");                      // pop lhs to RAX

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

        emit_line(ctx, "add rax, rcx");
    }
    else {
        error("Unsupported types for binary add operation");
    }
//    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    emit_line(ctx, "push rax");

}


void emit_translation_unit(EmitterContext * ctx, ASTNode * node) {
    emit_data_section_header(ctx);
    for (ASTNode_list_node * n = node->translation_unit.globals->head; n; n = n->next) {
        ASTNode * global_var = n->value;
        if (global_var->var_decl.init_expr) {
            // TODO write out correct emit_tree_node(ctx, n->value);
            char * data_directive = get_data_directive(global_var->ctype);
            emit_line(ctx, "%s: %s %d", global_var->var_decl.name, data_directive, global_var->var_decl.init_expr->int_value);
        }
    }

    emit_bss_section_header(ctx);
    for (ASTNode_list_node * n = node->translation_unit.globals->head; n; n = n->next) {
        ASTNode * global_var = n->value;
        if (!global_var->var_decl.init_expr) {
            // TODO write out correct emit_tree_node(ctx, n->value);
            char * reservation_directive = get_reservation_directive(global_var->ctype);
            emit_line(ctx, "%s: %s", global_var->var_decl.name, reservation_directive);
        }
    }

    emit_text_section_header(ctx);
    for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
        emit_tree_node(ctx, n->value);
    }
    emit_trailer(ctx);
}

void emit_assert_extension_statement(EmitterContext * ctx, ASTNode * node) {
    int label_pass = get_label_id(ctx);

    // evaluate expression
    emit_tree_node(ctx, node->expr_stmt.expr);

    // compare result in eax with 0
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "assert_pass", label_pass);

    // assert failed
    // print message
    emit_line(ctx, "mov rax, 1");
    emit_line(ctx, "mov rdi, 1");
    emit_line(ctx, "lea rsi, [rel assert_fail_msg]");
    emit_line(ctx, "mov rdx, 17");
    emit_line(ctx, "syscall");

    // exit
    emit_line(ctx, "mov rax, 60");
    emit_line(ctx, "mov rdi, 1");
    emit_line(ctx, "syscall");

    emit_label(ctx, "assert_pass", label_pass);

}

void emit_print_int_extension_call(EmitterContext * ctx, ASTNode * node) {
    // emit the expression storing it in EAX
    emit_tree_node(ctx, node->expr_stmt.expr);
    emit_line(ctx, "call print_int");
    ctx->emit_print_int_extension = true;
}

// emit_expr.
// generate code to eval the expression storing the final result in eax or rax
void emit_expr(EmitterContext * ctx, ASTNode * node) {
    switch (node->type) {
        case AST_INT_LITERAL:
            emit_line(ctx, "mov eax, %d", node->int_value);
            emit_line(ctx, "push rax");
            break;
        case AST_VAR_REF: {
            if (node->symbol->ctype->kind == CTYPE_ARRAY) {
                emit_line(ctx, "lea rax, [rbp-%d]", abs(node->symbol->info.var.offset));
                emit_line(ctx, "push rax");
            }
            else if (node->symbol->node->var_decl.is_global) {
                emit_line(ctx, "mov eax, [rel %s] ", node->symbol->name);
                emit_line(ctx, "push rax");
            } else {
                int offset = node->symbol->info.var.offset;
                emit_line(ctx, "mov %s, [rbp%+d]", reg_for_type(node->ctype), offset);
                emit_line(ctx, "push rax");
            }
            break;
        }
        case AST_UNARY_EXPR:
            emit_unary(ctx, node);
            break;
        case AST_BINARY_EXPR:
            emit_binary_expr(ctx, node);
            break;
        case AST_FUNCTION_CALL:
            emit_function_call(ctx, node);
            break;
        case AST_ARRAY_ACCESS:
            emit_addr(ctx, node);
            emit_line(ctx, "mov eax, [rcx]");
            break;
        case AST_CAST_EXPR:
            emit_expr(ctx, node->cast_expr.expr);     // eval inner expression
            if (node->ctype->kind == CTYPE_INT) {
                emit_line(ctx, "; cast to int: value already in eax");
            } else if (node->ctype->kind == CTYPE_CHAR) {
                emit_line(ctx, "movsx eax, al        ;cast to char");
            } else if (node->ctype->kind == CTYPE_LONG) {
                emit_line(ctx, "movsx rax, eax       ;cast to long");
            } else if (node->ctype->kind == CTYPE_SHORT) {
                emit_line(ctx, "movsx eax, ax        ; cast to short");
            } else {
                error("Unsupported cast type");
            }
            break;
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}


void emit_addr(EmitterContext * ctx, ASTNode * node) {
    switch (node->type) {
        case AST_VAR_DECL:
        case AST_VAR_REF: {

            char * label = create_variable_reference(ctx, node);
            emit_line(ctx, "lea rcx, %s", label);
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

            int base_offset = get_offset(ctx, node);
            base_offset = abs(base_offset);

            emit_expr(ctx, node->array_access.index);    // put result in eax
            emit_line(ctx, "imul eax, %d", node->ctype->size);  // scale index
            emit_line(ctx, "mov rcx, rbp");
            emit_line(ctx, "sub rcx, %d", base_offset);
            emit_line(ctx, "add rcx, rax");

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
        case BINOP_MUL:
            emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
   //         emit_line(ctx, "push rax");                     // push lhs result
            emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
            emit_line(ctx, "pop rcx");                      // pop lhs to ECX
            emit_line(ctx, "pop rax");
            emit_binary_op(ctx, node->binary.op);        // emit proper for op
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
    emit_tree_node(ctx, node->binary.lhs);
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    //rhs
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    // both true
    emit_line(ctx, "mov eax, 1");
    emit_jump(ctx, "jmp", "end", label_end);

    emit_label(ctx, "false", label_false);
    emit_line(ctx, "mov eax, 0");

    emit_label(ctx, "end", label_end);
}

void emit_logical_or(EmitterContext * ctx, ASTNode* node) {
    int label_true = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    // lhs
    emit_tree_node(ctx, node->binary.lhs);
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    // rhs
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    emit_label(ctx, "true", label_true);
    emit_line(ctx, "mov eax, 1");

    emit_label(ctx, "end", label_end);
}

void emit_binary_div(EmitterContext * ctx, ASTNode * node) {
    emit_tree_node(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_line(ctx, "push rax");                     // push lhs result
    emit_tree_node(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    emit_line(ctx, "mov ecx, eax");                 // move denominator to ecx
    emit_line(ctx, "pop rax");                      // restore numerator to eax
    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");
}

void emit_binary_mod(EmitterContext * ctx, ASTNode * node) {
    emit_tree_node(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_line(ctx, "push rax");                     // push lhs result
    emit_tree_node(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    emit_line(ctx, "mov ecx, eax");                 // move denominator to ecx
    emit_line(ctx, "pop rax");                      // restore numerator to eax
    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");               // divide eax by ecx. result goes to eax, remainder to edx
    emit_line(ctx, "mov eax, edx");           // move remainer in edx to eax
}

void emit_binary_comparison(EmitterContext * ctx, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_tree_node(ctx, node->binary.lhs);
    emit_line(ctx, "push rax");

    // eval right-hand side -> reult in eax

    emit_tree_node(ctx, node->binary.rhs);

    // restore lhs into rcx
    emit_line(ctx, "pop rcx");
    emit_line(ctx, "mov ecx, ecx");   // zero upper bits

    // compare rcx (lhs) with eax (rhs), cmp rcx, eax means rcx - eax
    emit_line(ctx, "cmp ecx, eax");

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
            emit_line(ctx, "neg eax");
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
            emit_line(ctx, "push rax");
            free(reference_label);
            break;
        }
        case UNARY_DEREF: {
            emit_expr(ctx, node->unary.operand);
            emit_line(ctx, "pop rax");
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
            emit_line(ctx, "push rax");
            break;
        }
        default:
            error("Unsupported unary op in emitter");
    }
}



void emit_if_statement(EmitterContext * ctx, ASTNode * node) {
    int id = get_label_id(ctx);
    // eval condition
    emit_tree_node(ctx, node->if_stmt.cond);
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

    emit_tree_node(ctx, node->switch_stmt.expr);
    emit_line(ctx, "push rax   ; save switch expression");

    push_loop_context(ctx, loop_start_label, loop_end_label);

    // start label
    emit_label_from_text(ctx, loop_start_label);
    // eval cond
    emit_tree_node(ctx, node->while_stmt.cond);
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
    emit_tree_node(ctx, node->do_while_stmt.body);
    emit_tree_node(ctx, node->do_while_stmt.expr);
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

    char * func_end_label = make_label_text("func_end", get_label_id(ctx));
    push_function_exit_context(ctx, func_end_label);

//    emit_text_section_header(ctx);
    int local_space = node->function_decl.size;

    emit_line(ctx, "%s:", node->function_decl.name);
    emit_line(ctx, "push rbp           ; creating stack frame");
    emit_line(ctx,  "mov rbp, rsp");

    if (local_space > 0) {
        int aligned_space = (local_space + 15) & ~15;
        emit_line(ctx, "sub rsp, %d        ; allocating space for locals", aligned_space);
    }

    if (node->function_decl.param_list) {
        for (const ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
            emit_var_declaration(ctx, n->value);
        }
    }

    emit_block(ctx, node->function_decl.body, false);

    emit_label_from_text(ctx, func_end_label);
    emit_line(ctx, "leave           ; function end");
    emit_line(ctx, "ret");

    pop_function_exit_context(ctx);
    free(func_end_label);
}

void emit_var_declaration(EmitterContext * ctx, ASTNode * node) {
    emit_line(ctx, "; emitting var declaration");
    if (node->var_decl.init_expr) {
        if (node->var_decl.init_expr->type == AST_INITIALIZER_LIST) {
            emit_line(ctx,"; initializing array");
            Symbol * symbol = node->symbol;
            ASTNode_list * init_items = node->var_decl.init_expr->initializer_list.items;
            for (int i=0;i<init_items->count;i++) {
                emit_line(ctx, "; initializing element %d", i);
                ASTNode * init_value = ASTNode_list_get(init_items, i);
                emit_expr(ctx, init_value);
                emit_line(ctx, "push rax");

                // TODO FIX emit_addr call
//                emit_addr(ctx, node);

//                emit_addr(ctx, node->array_access.base);

                int offset = symbol->info.array.offset + i*4;

                emit_line(ctx, "lea rcx, [rbp%+d]", offset);
                // if (symbol->info.array.offset >= 0) {
                //     emit_line(ctx, "lea rcx, [rbp - %d]\n", offset);
                // }
                // else {
                //     emit_line(ctx, "lea rcx, [rbp + %d]\n", -offset);
                // }

                emit_line(ctx, "pop rax");
                emit_line(ctx, "mov [rcx], eax");
            }
        }
        else {
            emit_line(ctx,"; initializing variable");
            emit_expr(ctx, node->var_decl.init_expr);
            emit_line(ctx, "push rax");
            emit_addr(ctx, node);
            emit_line(ctx, "pop rax");
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
}

void emit_assignment(EmitterContext * ctx, ASTNode* node) {
    emit_line(ctx, "; emitting assignment - LHS %s = RHS %s",
        get_ast_node_name(node->binary.lhs), get_ast_node_name(node->binary.rhs));
    // eval RHS -> rax then push
    emit_expr(ctx, node->binary.rhs);
    emit_line(ctx, "push rax");

    // eval LHS addr -> rcx
    emit_addr(ctx, node->binary.lhs);

    // pop RHS in rax
    emit_line(ctx, "pop rax");

    CType * ctype = node->binary.lhs->ctype;

    emit_line(ctx, "mov %s [rcx], %s",
        mem_size_for_type(ctype),
        reg_for_type(ctype));

    // if (ctype->kind == CTYPE_CHAR) {
    //     emit_line(ctx, "mov byte [rcx], eax\n");
    // } else if (ctype->kind == CTYPE_SHORT) {
    //     emit_line(ctx, "mov word [rcx], eax\n");
    // } else if (ctype->kind == CTYPE_INT) {
    //     emit_line(ctx, "mov dword [rcx], eax\n");
    // } else if (ctype->kind == CTYPE_LONG || ctype->kind == CTYPE_PTR) {
    //     emit_line(ctx, "mov qword [rcx], rax\n");
    // } else {
    //     error("Unsupported type in assignment");
    // }
}

void emit_add_assignment(EmitterContext * ctx, ASTNode * node) {

    // compute lhs address -> rcx
    emit_addr(ctx, node->binary.lhs);

    // push LHS address on stack
    emit_line(ctx, "push rcx");

    // load current value into eax
    emit_line(ctx, "mov eax, [rcx]");

    // save current RHS value in eax on stack
    emit_line(ctx, "push rax");

    // evaluate RHS into eax
    emit_expr(ctx, node->binary.rhs);

    // restore LHS into ecx
    emit_line(ctx, "pop rcx");

    // add RHS to LHS
    emit_line(ctx, "add eax, ecx");

    // restore LHS address
    emit_line(ctx, "pop rcx");

    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

    // add RHS to LHS

    // char * reference_label = create_variable_reference(ctx, node->binary.lhs);
    // emit_line(ctx, "mov eax, %s\n", reference_label);
    // emit_line(ctx, "push rax\n");
    // emit_tree_node(ctx, node->binary.rhs);
    // emit_line(ctx, "pop rcx\n");
    // emit_line(ctx, "add eax, ecx\n");
    // emit_line(ctx, "mov %s, eax\n", reference_label);
    // free(reference_label);
}

void emit_sub_assignment(EmitterContext * ctx, ASTNode * node) {

    // compute lhs address -> rcx
    emit_addr(ctx, node->binary.lhs);

    // push LHS address on stack
    emit_line(ctx, "push rcx");

    // load current value into eax
    emit_line(ctx, "mov eax, [rcx]");

    // save current RHS value in eax on stack
    emit_line(ctx, "push rax");

    // evaluate RHS into eax
    emit_expr(ctx, node->binary.rhs);

    emit_line(ctx, "mov ecx, eax");

    // restore LHS into ecx
    emit_line(ctx, "pop rax");

    // sub RHS from LHS
    emit_line(ctx, "sub eax, ecx");

    // restore LHS address
    emit_line(ctx, "pop rcx");

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
        emit_tree_node(ctx, node->for_stmt.init_expr);
    }

    // jump to condition check
    emit_jump_from_text(ctx, "jmp", condition_label);

    // loop body start
    emit_label_from_text(ctx, start_label);
    emit_block(ctx, node->for_stmt.body, false);

    // emit continue label
    emit_label_from_text(ctx, continue_label);

    // update expression
    if (node->for_stmt.update_expr) {
        emit_tree_node(ctx, node->for_stmt.update_expr);
    }

    // loop condition
    emit_label_from_text(ctx, condition_label);
    if (node->for_stmt.cond_expr) {
        emit_tree_node(ctx, node->for_stmt.cond_expr);
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
            emit_line(ctx, "push rax");
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
        emit_line(ctx, "add rsp, %d", arg_count*8);
    }
}

void emit_switch_dispatch(EmitterContext * ctx, ASTNode * node) {
    assert(node->type == AST_SWITCH_STMT);
    ASTNode * block = node->switch_stmt.stmt;
    assert(block->type == AST_BLOCK);

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
    assert(block->type == AST_BLOCK);

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

    emit_tree_node(ctx, node->switch_stmt.expr);
    emit_line(ctx, "push rax   ; save switch expression\n");

    push_switch_context(ctx, break_label);

    // first pass emit all comparisons and jumps
    emit_switch_dispatch(ctx, node);

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
            break;
        case AST_FUNCTION_CALL:
            emit_expr(ctx, node);
//            emit_function_call(ctx, node);
            break;
        case AST_EXPRESSION_STMT:
            emit_tree_node(ctx, node->expr_stmt.expr);
            break;
        case AST_ASSERT_EXTENSION_STATEMENT:
            emit_assert_extension_statement(ctx, node);
            break;
        case AST_PRINT_EXTENSION_STATEMENT:
            emit_print_int_extension_call(ctx, node);
            break;
        case AST_BLOCK:
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
        case AST_VAR_REF: {
            emit_expr(ctx, node);
            break;
        }
        case AST_ARRAY_ACCESS: {
            int offset = get_offset(ctx, node);
            emit_line(ctx, "mov eax, [rbp%+d]", offset);
            break;
        }
        case AST_INITIALIZER_LIST: {
            for (int i=0; i<node->initializer_list.items->count;i++) {

            }
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
        case AST_CAST_EXPR:
            emit_tree_node(ctx, node->cast_expr.expr);
            emit_cast(ctx, node->cast_expr.expr->ctype, node->cast_expr.target_type);
            break;
        default:
            error("Unhandled type %s\n", node->type);
            break;

    }
}

void emit_print_int_extension_function(EmitterContext * ctx) {
    int label_convert = get_label_id(ctx);
    int label_done = get_label_id(ctx);
    int label_buffer = get_label_id(ctx);
    int label_loop = get_label_id(ctx);

    emit_bss_section_header(ctx);
    emit_line(ctx, "buffer%d resb 20", label_buffer);

    emit_text_section_header(ctx);
    emit_line(ctx, "print_int:");
    emit_line(ctx, "; assumes integer to print is in eax");
    emit_line(ctx, "; converts and prints using syscall");
    emit_line(ctx, "mov rcx, buffer%d + 19", label_buffer);
    emit_line(ctx, "mov byte [rcx], 10");
    emit_line(ctx, "dec rcx");
    emit_line(ctx, "");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "convert", label_convert);
    emit_line(ctx, "mov byte [rcx], '0'");
    emit_line(ctx, "dec rcx");
    emit_jump(ctx, "jmp", "done", label_done);
    emit_line(ctx, "");
    emit_label(ctx, "convert", label_convert);
    emit_line(ctx, "xor edx, edx");
    emit_line(ctx, "mov ebx, 10");
    emit_label(ctx, "loop", label_loop);
    emit_line(ctx, "xor edx, edx");
    emit_line(ctx, "div ebx");
    emit_line(ctx, "add dl, '0'");
    emit_line(ctx, "mov [rcx], dl");
    emit_line(ctx, "dec rcx");
    emit_line(ctx, "test eax, eax");
    emit_jump(ctx, "jnz", "loop", label_loop);
    emit_line(ctx, "");
    emit_label(ctx, "done", label_done);
    emit_line(ctx, "lea rsi, [rcx + 1]");
    emit_line(ctx, "mov rdx, buffer%d + 20", label_buffer);
    emit_line(ctx, "sub rdx, rsi");
    emit_line(ctx, "mov rax, 1");
    emit_line(ctx, "mov rdi, 1");
    emit_line(ctx, "syscall");
    emit_line(ctx, "ret");

}

void emit(EmitterContext * ctx, ASTNode * translation_unit) {
   // populate_symbol_table(translation_unit);
    emit_tree_node(ctx, translation_unit);

    // emit referenced private functions
    if (ctx->emit_print_int_extension) {
        emit_print_int_extension_function(ctx);
    }

}