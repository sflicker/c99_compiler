#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "ctypes.h"
#include "ast.h"
#include "emitter.h"
#include "token.h"
#include "util.h"
#include "error.h"
#include "runtime_info_decorator.h"
#include "emitter_context.h"
#include "symbol.h"


// Register order for integer/pointer args in AMD64
// static const char* ARG_REGS[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
// const int ARG_REG_COUNT=6;

//bool emit_print_int_extension = false;

//static int label_id = 0;

// typedef struct FunctionExitContext {
//     char * exit_label;
//     struct FunctionExitContext * next;
// } FunctionExitContext;

//static FunctionExitContext * functionExitStack = NULL;

char * create_variable_reference(EmitterContext * ctx, ASTNode * node) {
    int size = 20;
    int offset = get_offset(ctx, node);
    char * label = malloc(size);
    snprintf(label, size, "[rbp%+d]", offset);
    return label;
}


// char * create_variable_reference(Address * addr) {
//     if (addr->kind == ADDR_STACK) {
//         int size = 20;
//         char * label = malloc(size);
//         snprintf(label, size, "[rbp%+d]", addr->stack_offset);
//         return label;
//     }
//     else if (addr->kind == ADDR_REGISTER) {
//         int size = 5;
//         char * label = malloc(size);
//         snprintf(label, size, "%s", ARG_REGS[addr->reg_index]);
//         return label;
//     }
//     else {
//         error("Variable Reference Addresses Must Be Assigned Before Usage\n");
//     }
//     return NULL;
// }

// void push_function_exit_context(EmitterContext * ctx, const char * exit_label) {
//     FunctionExitContext * functionCtx = malloc(sizeof(FunctionExitContext));
//     functionCtx->exit_label = strdup(exit_label);
//     functionCtx->next = ctx->functionExitStack;
//     ctx->functionExitStack = functionCtx;
// }

// void pop_function_exit_context(EmitterContext * ctx) {
//     if (functionExitStack) {
//         FunctionExitContext * old = functionExitStack;
//         functionExitStack = old->next;
//         free(old->exit_label);
//         free(old);
//     }
// }

// char * get_function_exit_label() {
//     if (functionExitStack) {
//         return functionExitStack->exit_label;
//     }
//     return NULL;
// }
// typedef struct SwitchContext {
//     const char * break_label;
//     struct SwitchContext * next;
// } SwitchContext;
//
// static SwitchContext * switch_stack = NULL;

// void push_switch_context(const char * break_label) {
//     SwitchContext * ctx = malloc(sizeof(SwitchContext));
//     ctx->break_label = strdup(break_label);
//     ctx->next = switch_stack;
//     switch_stack = ctx;
// }
//
// void pop_switch_context() {
//     if (switch_stack) {
//         SwitchContext * old = switch_stack;
//         switch_stack = old->next;
//         free(old);
//     }
// }

// typedef struct LoopContext {
//     const char * start_label;
//     const char * end_label;
//     struct LoopContext * next;
// } LoopContext;

//static LoopContext * loop_stack = NULL;

// void push_loop_context(const char * start_label, const char * end_label) {
//     LoopContext * ctx = malloc(sizeof(LoopContext));
//     ctx->start_label = start_label;
//     ctx->end_label = end_label;
//     ctx->next = loop_stack;
//     loop_stack = ctx;
// }
//
// void pop_loop_context() {
//     if (loop_stack) {
//         LoopContext * old = loop_stack;
//         loop_stack = old->next;
//         free(old);
//     }
// }

// const char * current_switch_break_label(EmitterContext * ctx) {
//     if (ctx->switch_stack) {
//         return ctx->switch_stack->break_label;
//     }
//     return NULL;  // error break outside switch
// }

void emit_line(EmitterContext * ctx, const char* fmt, ...) {
    va_list args;

    // --- 1. Write to the file
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);

    // --- 2. Echo to stdout (re-initialize args)
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

void emit_header(EmitterContext * ctx) {
    emit_line(ctx, "section .text\n");
    emit_line(ctx, "global main\n");
    emit_line(ctx, "\n");
}

void emit_trailer(EmitterContext * ctx) {
    emit_line(ctx, "\n");
    emit_line(ctx, "section .rodata\n");
    emit_line(ctx, "assert_fail_msg: db \"Assertion failed!\", 10\n");
}

void emit_text_section_header(EmitterContext * ctx) {
    emit_line(ctx, "\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, ";   SECTION: Text (Code)\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, "\n");
    emit_line(ctx, "section .text\n");
    emit_line(ctx, "\n");
}

void emit_data_section_header(EmitterContext * ctx) {
    emit_line(ctx, "\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, ";   SECTION: Data (Initialized globals/strings\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, "\n");
    emit_line(ctx, "section .data\n");
    emit_line(ctx, "\n");
}

void emit_bss_section_header(EmitterContext * ctx) {
    emit_line(ctx, "\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, ";   SECTION: BSS (Uninitialized buffers)\n");
    emit_line(ctx, ";---------------------------------------\n");
    emit_line(ctx, "\n");
    emit_line(ctx, "section .bss\n");
    emit_line(ctx, "\n");
}

char * make_label_text(const char * prefix, int num) {
    size_t buffer_size = strlen(prefix) + 20;
    char * label = malloc(buffer_size);
    snprintf(label, buffer_size, "%s%d", prefix, num);
    return label;
}

void emit_label(EmitterContext * ctx, const char * prefix, int num) {
    emit_line(ctx, ".L%s%d:\n", prefix, num);
}

void emit_label_from_text(EmitterContext *ctx, const char * label) {
    emit_line(ctx, ".L%s:\n", label);
}

void emit_jump(EmitterContext * ctx, const char * op, const char * prefix, int num) {
    emit_line(ctx, "%s .L%s%d\n", op, prefix, num);
}

void emit_jump_from_text(EmitterContext * ctx, const char * op, const char * label) {
    emit_line(ctx, "%s .L%s\n", op, label);
}

void emit_assert_extension_statement(EmitterContext * ctx, ASTNode * node) {
    int label_pass = get_label_id(ctx);

    // evaluate expression
    emit_tree_node(ctx, node->expr_stmt.expr);

    // compare result in eax with 0
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "jne", "assert_pass", label_pass);

    // assert failed
    // print message
    emit_line(ctx, "mov rax, 1\n");
    emit_line(ctx, "mov rdi, 1\n");
    emit_line(ctx, "lea rsi, [rel assert_fail_msg]\n");
    emit_line(ctx, "mov rdx, 17\n");
    emit_line(ctx, "syscall\n");

    // exit
    emit_line(ctx, "mov rax, 60\n");
    emit_line(ctx, "mov rdi, 1\n");
    emit_line(ctx, "syscall\n");

    emit_label(ctx, "assert_pass", label_pass);

}

void emit_print_int_extension_call(EmitterContext * ctx, ASTNode * node) {
    // emit the expression storing it in EAX
    emit_tree_node(ctx, node->expr_stmt.expr);
    emit_line(ctx, "call print_int\n");
    ctx->emit_print_int_extension = true;
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
        case BINOP_SUB:
        case BINOP_MUL:
            emit_tree_node(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
            emit_line(ctx, "push rax\n");                     // push lhs result
            emit_tree_node(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
            emit_line(ctx, "pop rcx\n");                      // pop lhs to ECX
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
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "je", "false", label_false);

    //rhs
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "je", "false", label_false);

    // both true
    emit_line(ctx, "mov eax, 1\n");
    emit_jump(ctx, "jmp", "end", label_end);

    emit_label(ctx, "false", label_false);
    emit_line(ctx, "mov eax, 0\n");

    emit_label(ctx, "end", label_end);
}

void emit_logical_or(EmitterContext * ctx, ASTNode* node) {
    int label_true = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    // lhs
    emit_tree_node(ctx, node->binary.lhs);
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "jne", "true", label_true);

    // rhs
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "jne", "true", label_true);

    emit_label(ctx, "true", label_true);
    emit_line(ctx, "mov eax, 1\n");

    emit_label(ctx, "end", label_end);
}

void emit_binary_div(EmitterContext * ctx, ASTNode * node) {
    emit_tree_node(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_line(ctx, "push rax\n");                     // push lhs result
    emit_tree_node(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    emit_line(ctx, "mov ecx, eax\n");                 // move denominator to ecx
    emit_line(ctx, "pop rax\n");                      // restore numerator to eax
    emit_line(ctx, "cdq\n");
    emit_line(ctx, "idiv ecx\n");
}

void emit_binary_mod(EmitterContext * ctx, ASTNode * node) {
    emit_tree_node(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
    emit_line(ctx, "push rax\n");                     // push lhs result
    emit_tree_node(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
    emit_line(ctx, "mov ecx, eax\n");                 // move denominator to ecx
    emit_line(ctx, "pop rax\n");                      // restore numerator to eax
    emit_line(ctx, "cdq\n");
    emit_line(ctx, "idiv ecx\n");               // divide eax by ecx. result goes to eax, remainder to edx
    emit_line(ctx, "mov eax, edx\n");           // move remainer in edx to eax
}

void emit_binary_comparison(EmitterContext * ctx, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_tree_node(ctx, node->binary.lhs);
    emit_line(ctx, "push rax\n");

    // eval right-hand side -> reult in eax

    emit_tree_node(ctx, node->binary.rhs);

    // restore lhs into rcx
    emit_line(ctx, "pop rcx\n");
    emit_line(ctx, "mov ecx, ecx\n");   // zero upper bits

    // compare rcx (lhs) with eax (rhs), cmp rcx, eax means rcx - eax
    emit_line(ctx, "cmp ecx, eax\n");

    // emit proper setX based on operator type
    switch (node->binary.op) {
        case BINOP_EQ:
            emit_line(ctx, "sete al\n");
            break;

        case BINOP_NE:
            emit_line(ctx, "setne al\n");
            break;

        case BINOP_LT:
            emit_line(ctx, "setl al\n");
            break;

        case BINOP_LE:
            emit_line(ctx, "setle al\n");
            break;

        case BINOP_GT:
            emit_line(ctx, "setg al\n");
            break;

        case BINOP_GE:
            emit_line(ctx, "setge al\n");
            break;

        default:
            error("Unsupported comparison type in codegen.");
    }

    // zero-extend result to full eax
    emit_line(ctx, "movzx eax, al\n");

}


void emit_binary_op(EmitterContext * ctx, BinaryOperator op) {
    switch(op) {
        case BINOP_ADD:
            emit_line(ctx, "add eax, ecx\n");
            break;
        case BINOP_SUB:
//            emit_line(out, "sub eax, ecx\n");
              emit_line(ctx, "sub ecx, eax\n");
              emit_line(ctx, "mov eax, ecx\n");
            break;
        case BINOP_MUL:
            emit_line(ctx, "imul eax, ecx\n");
            break;
        default:
            error("Unsupported binary operator: %s", token_type_name(op));
    }
}

void emit_unary(EmitterContext * ctx, ASTNode * node) {
    switch (node->unary.op) {
        case UNARY_NEGATE:
            emit_tree_node(ctx, node->unary.operand);
            emit_line(ctx, "neg eax\n");
            break;
        case UNARY_PLUS:
            // noop
            break;
        case UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_tree_node(ctx, node->unary.operand);
            emit_line(ctx, "cmp eax, 0\n");
            emit_line(ctx, "sete al\n");
            emit_line(ctx, "movzx eax, al\n");
            break;
        case UNARY_PRE_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
//            emit_line(out, "mov eax, [rbp%+d]\n", offset);            
            emit_line(ctx, "mov eax, %s\n", reference_label);
            emit_line(ctx, "add eax, 1\n");
            emit_line(ctx, "mov %s, eax\n", reference_label);
            break;
        }
        case UNARY_PRE_DEC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            //int offset = node->unary.operand->var_ref->offset;
            emit_line(ctx, "mov eax, %s\n", reference_label);
            emit_line(ctx, "sub eax, 1\n");
            emit_line(ctx, "mov %s, eax\n", reference_label);
        break;
        }
        case UNARY_POST_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s\n", reference_label);
            emit_line(ctx, "mov ecx, eax\n");
            emit_line(ctx, "add eax, 1\n");
            emit_line(ctx, "mov %s, eax\n", reference_label);
            emit_line(ctx, "mov eax, ecx\n");
            break;
        }
        case UNARY_POST_DEC: {
//            int offset = lookup_symbol(node->unary.operand->var_expr.name);
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s\n", reference_label);
            emit_line(ctx, "mov ecx, eax\n");
            emit_line(ctx, "sub eax, 1\n");
            emit_line(ctx, "mov %s, eax\n", reference_label);
            emit_line(ctx, "mov eax, ecx\n");
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
    emit_line(ctx, "cmp eax, 0\n");

    if (node->if_stmt.else_stmt) {
        emit_line(ctx, "je .Lelse%d\n", id);  // jump to else if false
        emit_tree_node(ctx, node->if_stmt.then_stmt);
        emit_line(ctx, "jmp .Lend%d\n", id);  // jump to end over else
        emit_label(ctx, "else", id);
        emit_tree_node(ctx, node->if_stmt.else_stmt);
    }
    else {
        emit_line(ctx, "je .Lend%d\n", id);  // skip over if false
        emit_tree_node(ctx, node->if_stmt.then_stmt);
    }
    emit_label(ctx, "end", id);
}

void emit_while_statement(EmitterContext * ctx, ASTNode * node) {

    char * loop_start_label = make_label_text("while_start", get_label_id(ctx));
    char * loop_end_label = make_label_text("while_end", get_label_id(ctx));

    emit_tree_node(ctx, node->switch_stmt.expr);
    emit_line(ctx, "push rax   ; save switch expression\n");

    push_loop_context(ctx, loop_start_label, loop_end_label);
    
    //int id = label_id++;
    
    // start label
    emit_label_from_text(ctx, loop_start_label);
    // eval cond
    emit_tree_node(ctx, node->while_stmt.cond);
    // cmp to zero
    emit_line(ctx, "cmp eax, 0\n");
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
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "jne", "do_while_start", id);
    emit_label(ctx, "do_while_end", id);
}

void emit_block(EmitterContext * ctx, ASTNode * node, bool enterNewScope) {

    for (const ASTNode_list_node * n = node->block.statements->head; n; n = n->next) {
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

    emit_text_section_header(ctx);
//    int local_space = runtime_info(node)->size;
    int local_space = node->function_decl.size;

    emit_line(ctx, "%s:\n", node->function_decl.name);

    emit_line(ctx, "push rbp\n");
    emit_line(ctx,  "mov rbp, rsp\n");

    if (local_space > 0) {
        emit_line(ctx, "sub rsp, %d\n", local_space);
    }

    if (node->function_decl.param_list) {
        for (const ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
            emit_var_declaration(ctx, n->value);
        }
    }

    //TODO FIX THIS
    // if (node->function_decl.param_list) {
    //     for (struct node_list * param = node->function_decl.param_list->param_list.node_list; param != NULL; param = param->next) {
    //         ASTNode * var_decl = param->node;
    //         emit_var_declaration(out, var_decl);
    //     }
    // }

    emit_block(ctx, node->function_decl.body, false);

    emit_label_from_text(ctx, func_end_label);
    emit_line(ctx, "leave\n");
    emit_line(ctx, "ret\n");

    pop_function_exit_context(ctx);
    free(func_end_label);
}

void emit_var_declaration(EmitterContext * ctx, ASTNode * node) {
    if (node->var_decl.init_expr) {
        char * reference_label = create_variable_reference(ctx, node);
        emit_tree_node(ctx, node->var_decl.init_expr);
        emit_line(ctx, "mov %s, eax\n", reference_label);
    }
}

void emit_assignment(EmitterContext * ctx, ASTNode* node) {
    emit_tree_node(ctx, node->binary.rhs);
    char * reference_label = create_variable_reference(ctx, node->binary.lhs);
    emit_line(ctx, "mov %s, eax\n", reference_label);
}

void emit_add_assignment(EmitterContext * ctx, ASTNode * node) {
    char * reference_label = create_variable_reference(ctx, node->binary.lhs);
    emit_line(ctx, "mov eax, %s\n", reference_label);
    emit_line(ctx, "push rax\n");
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "pop rcx\n");
    emit_line(ctx, "add eax, ecx\n");
    emit_line(ctx, "mov %s, eax\n", reference_label);
}

void emit_sub_assignment(EmitterContext * ctx, ASTNode * node) {
    char * reference_label  = create_variable_reference(ctx, node->binary.lhs);
    emit_tree_node(ctx, node->binary.rhs);
    emit_line(ctx, "mov ecx, eax\n");
    emit_line(ctx, "mov eax, %s\n", reference_label);
    emit_line(ctx, "sub eax, ecx\n");
    emit_line(ctx, "mov %s, eax\n", reference_label);
}

void emit_for_statement(EmitterContext * ctx, ASTNode * node) {

    char * start_label = make_label_text("for_start", get_label_id(ctx));
    char * end_label = make_label_text("for_end", get_label_id(ctx));
    char * condition_label = make_label_text("for_condition", get_label_id(ctx));
    char * continue_label = make_label_text("for_continue", get_label_id(ctx));

    push_loop_context(ctx, continue_label, end_label);

    // int label_start = label_id++;
    // int label_cond = label_id++;
    // int label_end = label_id++;

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
        emit_line(ctx, "cmp eax, 0\n");
        emit_jump_from_text(ctx, "je", end_label);     // exit if false
    }

    emit_jump_from_text(ctx, "jmp", start_label);
    
    // end/break label
    emit_label_from_text(ctx, end_label);

    pop_loop_context(ctx);

//    exit_scope();

    free(start_label);
    free(end_label);
    free(condition_label);
    free(continue_label);

}

void emit_pass_argument(EmitterContext * ctx, CType * type, ASTNode * node) {
    emit_tree_node(ctx, node);
    char * reference_label = create_variable_reference(ctx, node);
    switch(type->size) {
        case 1:
            emit_line(ctx, "mov byte %s, al\n", reference_label);
            break;
        case 2:
            emit_line(ctx, "mov word %s, ax\n", reference_label);
            break;
        case 4:
            emit_line(ctx, "mov dword %s, eax\n", reference_label);
            break;
        case 8:
            emit_line(ctx, "mov qword %s, rax\n", reference_label);
            break;
        default:
            error("Unsupported type size %d in emit_pass_argument\n", type->size);
    }
}

void emit_function_call(EmitterContext * ctx, ASTNode * node) {
    // if the call has arguments
    // first get a reversed list
    // then emit each arg then push it
    //struct node_list * reversed_list = NULL;

    //int arg_count=0;
    if (node->function_call.arg_list) {
     //   reverse_ASTNode_list(node->function_call.arg_list);

        for (int i = node->function_call.arg_list->count - 1; i >= 0; i--) {
            ASTNode * argNode = ASTNode_list_get(node->function_call.arg_list, i);
            emit_tree_node(ctx, argNode);
            emit_line(ctx, "push rax\n");
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

    //    reverse_ASTNode_list(node->function_call.arg_list);
    }

//     if (node->function_call.argument_expression_list) {
//  //       reversed_list = reverse_list(node->function_call.argument_expression_list);
//         reversed_list = node->function_call.argument_expression_list;
//         for (struct node_list * arg = reversed_list;arg != NULL; arg = arg->next) {
//             ASTNode * argNode = (ASTNode*)arg;
//             emit_pass_argument(out, argNode->var_decl.var_type, argNode->var_decl.offset, argNode);
//  //           emit_tree_node(out, arg->node);
// //            emit_line(out, "mov [rbp+%d], rax\n", arg->node.var_decl.offset);
//         //    emit_line(out, "push rax\n");
//         }
//     }

    // // call the function
    emit_line(ctx, "call %s\n", node->function_call.name);

    // // clean up arguments
    // int total_arg_size = get_node_list_count(reversed_list) * 8;
    // if (total_arg_size > 0) {
    //     emit_line(out, "add rsp, %d\n", total_arg_size);
    // }

    // if (reversed_list) {
    //     free_node_list(reversed_list);
    // }
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
            emit_line(ctx, "mov rax, [rsp]\n");
            emit_line(ctx, "cmp rax, %d\n", statement->case_stmt.constExpression->int_value);
//            emit_line(ctx, "je %s\n", statement->case_stmt.label);
//            emit_line(ctx, "je %s\n",  runtime_info(statement)->label);
            emit_line(ctx, "je %s\n",  statement->case_stmt.label);
        }
        else if (statement->type == AST_DEFAULT_STMT) {
            // statement->default_stmt.label = make_label_text("default", label_id++);
            // emit_line(ctx, "jmp %s\n", statement->default_stmt.label);
            // runtime_info(statement)->label = make_label_text("default", get_label_id(ctx));
            // emit_line(ctx, "jmp %s\n", runtime_info(statement)->label);
            node->default_stmt.label = make_label_text("default", get_label_id(ctx));
            emit_line(ctx, "jmp %s\n", node->default_stmt.label);
        }
    }
}

void emit_switch_bodies(EmitterContext * ctx, ASTNode * node) {
        assert(node->type == AST_SWITCH_STMT);
    ASTNode * block = node->switch_stmt.stmt;
    assert(block->type == AST_BLOCK);

    // for (int i=0;i<block->block.count;i++) {
    //     ASTNode * statement = block->block.statements[i];
    //     if (statement->type == AST_CASE_STMT) {
    //         emit_line(out, "%s:\n", statement->case_stmt.label);
    //         emit_tree_node(out, statement->case_stmt.stmt);
    //     }
    //     else if (statement->type == AST_DEFAULT_STMT) {
    //         emit_line(out, "%s:\n", statement->default_stmt.label);
    //         emit_tree_node(out, statement->default_stmt.stmt);
    //     }
    // }
}

void emit_switch_statement(EmitterContext * ctx, ASTNode * node) {
    assert(node->type == AST_SWITCH_STMT);
    int label_end = get_label_id(ctx);

    char * break_label = make_label_text("switch_end", label_end);

    emit_tree_node(ctx, node->switch_stmt.expr);
    emit_line(ctx, "push rax   ; save switch expression\n");

    push_switch_context(ctx, break_label);

    // first pass emit all comparisons and jumps
//    emit_switch_dispatch(out, node->switch_stmt.stmt);
    emit_switch_dispatch(ctx, node);

    // second pass emit all labels and bodies
//    emit_switch_bodies(out, node->switch_stmt.stmt);
    emit_switch_bodies(ctx, node);

    emit_line(ctx, "%s\n", break_label);
    // TODO MAY NEED TO EMIT A STACK restore
    //emit_line(out, "add rsp, 8  ; restore stack\n");

    pop_switch_context(ctx);

    free(break_label);
}

void emit_case_statement(EmitterContext * ctx, ASTNode * node) {
    int case_label_id = get_label_id(ctx);
    char * case_label = make_label_text("case", case_label_id);
    node->case_stmt.label = strdup(case_label);
//    runtime_info(node)->label = strdup(case_label);

    // load switch value back from the stack
    emit_line(ctx, "mov rax, [rsp] ; reload switch expr\n");
    
    // emit the jmp to the case body
    emit_line(ctx, "cmp rax, %d\n", node->case_stmt.constExpression->int_value);
    emit_line(ctx, "je %s\n", case_label);

    // emit the case body
    emit_line(ctx, "%s\n", node->case_stmt.label);
///    emit_line(ctx, "%s\n", runtime_info(node)->label);
    emit_tree_node(ctx, node->case_stmt.stmt);
    // emit jump to break

    emit_line(ctx, "jmp %s\n", current_switch_break_label(ctx));

    free(case_label);
}

const char * get_break_label(EmitterContext * ctx) {
    if (ctx->loop_stack) return ctx->loop_stack->end_label;
    return current_switch_break_label(ctx);
    // if (switch_stack) return switch_stack->break_label;
    // return NULL;
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

void emit_tree_node(EmitterContext * ctx, ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {
            emit_header(ctx);
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                 emit_tree_node(ctx, n->value);
            }
            emit_trailer(ctx);
            break;
        }
        case AST_FUNCTION_DECL:
            emit_function(ctx, node);
            break;
        case AST_VAR_DECL:
            emit_var_declaration(ctx, node);
            break;
        // case AST_ASSIGNMENT:
        //     emit_assignment(out, node);
        //     break;
        // case AST_COMPOUND_ADD_ASSIGN:
        //     emit_add_assignment(out, node);
        //     break;
        // case AST_COMPOUND_SUB_ASSIGN:
        //     emit_sub_assignment(out, node);
        //     break;
        case AST_RETURN_STMT:
            emit_tree_node(ctx, node->return_stmt.expr);
            if (ctx->functionExitStack && ctx->functionExitStack->exit_label) {
                emit_jump_from_text(ctx, "jmp", ctx->functionExitStack->exit_label);
            }
            // emit_line(out, "leave\n");
            // emit_line(out, "ret\n");
            break;
        case AST_FUNCTION_CALL:
            emit_function_call(ctx, node);
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
            emit_binary_expr(ctx, node);
            break;


        // case AST_DIV: {
        //     emit_binary_div(out, node);
        //     emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
        //     emit_line(out, "push rax\n");                     // push lhs result
        //     emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
        //     emit_line(out, "mov ecx, eax\n");                 // move denominator to ecx
        //     emit_line(out, "pop rax\n");                      // restore numerator to eax
        //     emit_line(out, "cdq\n");
        //     emit_line(out, "idiv ecx\n");
        //     break;
        // }
        // case AST_MOD: {
        //     emit_binary_mod(out, node);
        //     emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
        //     emit_line(out, "push rax\n");                     // push lhs result
        //     emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
        //     emit_line(out, "mov ecx, eax\n");                 // move denominator to ecx
        //     emit_line(out, "pop rax\n");                      // restore numerator to eax
        //     emit_line(out, "cdq\n");
        //     emit_line(out, "idiv ecx\n");               // divide eax by ecx. result goes to eax, remainder to edx
        //     emit_line(out, "mov eax, edx\n");           // move remainer in edx to eax
        //     break;
        // }
        // case AST_ADD:
        // case AST_SUB:
        // case AST_MUL:
        //     emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
        //     emit_line(out, "push rax\n");                     // push lhs result
        //     emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
        //     emit_line(out, "pop rcx\n");                      // pop lhs to ECX
        //     emit_binary_op(out, node->binary.op);        // emit proper for op
        // break;

        // case AST_EQUAL:
        // case AST_NOT_EQUAL:
        // case AST_LESS_EQUAL:
        // case AST_LESS_THAN:
        // case AST_GREATER_EQUAL:
        // case AST_GREATER_THAN:
        //     emit_binary_comparison(out, node);
        //     break;

        // case AST_UNARY_POST_INC:
        // case AST_UNARY_POST_DEC:
        // case AST_UNARY_PRE_INC:
        // case AST_UNARY_PRE_DEC:
        // case AST_UNARY_NEGATE:
        // case AST_UNARY_NOT:
        // case AST_UNARY_PLUS:
        // case AST_UNARY_EXPR:
        emit_unary(ctx, node);
            break;

        case AST_UNARY_EXPR:
            emit_unary(ctx, node);
            break;


        // case AST_LOGICAL_AND:
        //     emit_logical_and(out, node);
        //     break;
        //
        // case AST_LOGICAL_OR:
        //     emit_logical_or(out, node);
        //     break;

        case AST_INT_LITERAL:
            emit_line(ctx, "mov eax, %d\n", node->int_value);
            break;
        case AST_VAR_REF:
//            int offset = runtime_info(node)->offset;
//            int offset = node->var_ref.addr.stack_offset;
            int offset = node->symbol->info.var.offset;
            emit_line(ctx, "mov eax, [rbp%+d]\n", offset);
            break;
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
    emit_line(ctx, "buffer%d resb 20\n", label_buffer);

    emit_text_section_header(ctx);
    emit_line(ctx, "print_int:\n");
    emit_line(ctx, "; assumes integer to print is in eax\n");
    emit_line(ctx, "; converts and prints using syscall\n");
    emit_line(ctx, "mov rcx, buffer%d + 19\n", label_buffer);
    emit_line(ctx, "mov byte [rcx], 10\n");
    emit_line(ctx, "dec rcx\n");
    emit_line(ctx, "\n");
    emit_line(ctx, "cmp eax, 0\n");
    emit_jump(ctx, "jne", "convert", label_convert);
    emit_line(ctx, "mov byte [rcx], '0'\n");
    emit_line(ctx, "dec rcx\n");
    emit_jump(ctx, "jmp", "done", label_done);
    emit_line(ctx, "\n");
    emit_label(ctx, "convert", label_convert);
    emit_line(ctx, "xor edx, edx\n");
    emit_line(ctx, "mov ebx, 10\n");
    emit_label(ctx, "loop", label_loop);
    emit_line(ctx, "xor edx, edx\n");
    emit_line(ctx, "div ebx\n");
    emit_line(ctx, "add dl, '0'\n");
    emit_line(ctx, "mov [rcx], dl\n");
    emit_line(ctx, "dec rcx\n");
    emit_line(ctx, "test eax, eax\n");
    emit_jump(ctx, "jnz", "loop", label_loop);
    emit_line(ctx, "\n");
    emit_label(ctx, "done", label_done);
    emit_line(ctx, "lea rsi, [rcx + 1]\n");
    emit_line(ctx, "mov rdx, buffer%d + 20\n", label_buffer);
    emit_line(ctx, "sub rdx, rsi\n");
    emit_line(ctx, "mov rax, 1\n");
    emit_line(ctx, "mov rdi, 1\n");
    emit_line(ctx, "syscall\n");
    emit_line(ctx, "ret\n");

}

void emit(EmitterContext * ctx, ASTNode * translation_unit) {
 //   FILE * out = fopen(output_file, "w");
    populate_symbol_table(translation_unit);
    emit_tree_node(ctx, translation_unit);

    // emit referenced private functions
    if (ctx->emit_print_int_extension) {
        emit_print_int_extension_function(ctx);
    }

}