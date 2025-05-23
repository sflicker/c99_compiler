#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "emitter.h"
#include "token.h"
#include "util.h"

void emit_tree_node(FILE * out, ASTNode * node);
void emit_var_declaration(FILE *out, ASTNode * node);

bool emit_print_int_extension = false;

static int label_id = 0;

typedef struct SwitchContext {
    const char * break_label;
    struct SwitchContext * next;
} SwitchContext;

static SwitchContext * switch_stack = NULL;

void push_switch_context(const char * break_label) {
    SwitchContext * ctx = malloc(sizeof(SwitchContext));
    ctx->break_label = break_label;
    ctx->next = switch_stack;
    switch_stack = ctx;
}

void pop_switch_context() {
    if (switch_stack) {
        SwitchContext * old = switch_stack;
        switch_stack = old->next;
        free(old);
    }
}

const char * current_switch_break_label() {
    if (switch_stack) {
        return switch_stack->break_label;
    }
    return NULL;  // error break outside switch
}

void emit_line(FILE* out, const char* fmt, ...) {
    va_list args;

    // --- 1. Write to the file
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    // --- 2. Echo to stdout (re-initialize args)
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

void emit_header(FILE* out) {
    emit_line(out, "section .text\n");
    emit_line(out, "global main\n");
    emit_line(out, "\n");
}

void emit_trailer(FILE* out) {
    emit_line(out, "\n");
    emit_line(out, "section .rodata\n");
    emit_line(out, "assert_fail_msg: db \"Assertion failed!\", 10\n");
}

void emit_text_section_header(FILE * out) {
    emit_line(out, "\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, ";   SECTION: Text (Code)\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, "\n");
    emit_line(out, "section .text\n");
    emit_line(out, "\n");
}

void emit_data_section_header(FILE * out) {
    emit_line(out, "\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, ";   SECTION: Data (Initialized globals/strings\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, "\n");
    emit_line(out, "section .data\n");
    emit_line(out, "\n");
}

void emit_bss_section_header(FILE * out) {
    emit_line(out, "\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, ";   SECTION: BSS (Uninitialized buffers)\n");
    emit_line(out, ";---------------------------------------\n");
    emit_line(out, "\n");
    emit_line(out, "section .bss\n");
    emit_line(out, "\n");
}

const char * make_label_text(const char * prefix, int num) {
    size_t buffer_size = strlen(prefix) + 20;
    char * label = malloc(buffer_size);
    snprintf(label, buffer_size, ".L%s%d", prefix, num);
    return label;
}

void emit_label(FILE * out, const char * prefix, int num) {
    emit_line(out, ".L%s%d:\n", prefix, num);
}

void emit_jump(FILE * out, const char * op, const char * prefix, int num) {
    emit_line(out, "%s .L%s%d\n", op, prefix, num);
}

void emit_assert_extension_statement(FILE * out, ASTNode * node) {
    int label_pass = label_id++;

    // evaluate expression
    emit_tree_node(out, node->expr_stmt.expr);

    // compare result in eax with 0
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "assert_pass", label_pass);

    // assert failed
    // print message
    emit_line(out, "mov rax, 1\n");
    emit_line(out, "mov rdi, 1\n");
    emit_line(out, "lea rsi, [rel assert_fail_msg]\n");
    emit_line(out, "mov rdx, 17\n");
    emit_line(out, "syscall\n");

    // exit
    emit_line(out, "mov rax, 60\n");
    emit_line(out, "mov rdi, 1\n");
    emit_line(out, "syscall\n");

    emit_label(out, "assert_pass", label_pass);

}

void emit_print_extension_statement(FILE * out, ASTNode * node) {
    // emit the expression storing it in EAX
    emit_tree_node(out, node->expr_stmt.expr);
    emit_line(out, "call print_int\n");
    emit_print_int_extension = true;
}

void emit_logical_and(FILE * out, ASTNode * node) {
    int label_false = label_id++;
    int label_end = label_id++;

    //lhs 
    emit_tree_node(out, node->binary.lhs);
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "je", "false", label_false);

    //rhs
    emit_tree_node(out, node->binary.rhs);
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "je", "false", label_false);

    // both true
    emit_line(out, "mov eax, 1\n");
    emit_jump(out, "jmp", "end", label_end);

    emit_label(out, "false", label_false);
    emit_line(out, "mov eax, 0\n");

    emit_label(out, "end", label_end);
}

void emit_logical_or(FILE* out, ASTNode* node) {
    int label_true = label_id++;
    int label_end = label_id++;

    // lhs
    emit_tree_node(out, node->binary.lhs);
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "true", label_true);

    // rhs
    emit_tree_node(out, node->binary.rhs);
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "true", label_true);

    emit_label(out, "true", label_true);
    emit_line(out, "mov eax, 1\n");

    emit_label(out, "end", label_end);
}


void emit_binary_comparison(FILE * out, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_tree_node(out, node->binary.lhs);
    emit_line(out, "push rax\n");

    // eval right-hand side -> reult in eax

    emit_tree_node(out, node->binary.rhs);

    // restore lhs into rcx
    emit_line(out, "pop rcx\n");
    emit_line(out, "mov ecx, ecx\n");   // zero upper bits

    // compare rcx (lhs) with eax (rhs), cmp rcx, eax means rcx - eax
    emit_line(out, "cmp ecx, eax\n");

    // emit proper setX based on operator type
    switch (node->type) {
        case AST_EQUAL:
            emit_line(out, "sete al\n");
            break;

        case AST_NOT_EQUAL:
            emit_line(out, "setne al\n");
            break;

        case AST_LESS_THAN:
            emit_line(out, "setl al\n");
            break;

        case AST_LESS_EQUAL:
            emit_line(out, "setle al\n");
            break;

        case AST_GREATER_THAN:
            emit_line(out, "setg al\n");
            break;

        case AST_GREATER_EQUAL:
            emit_line(out, "setge al\n");
            break;

        default:
            fprintf(stderr, "Unsupported comparison type in codegen.\n");
            exit(1);

    }

    // zero-extend result to full eax
    emit_line(out, "movzx eax, al\n");

}


void emit_binary_op(FILE * out, ASTNodeType op) {
    switch(op) {
        case AST_ADD:
            emit_line(out, "add eax, ecx\n");
            break;
        case AST_SUB:
//            emit_line(out, "sub eax, ecx\n");
              emit_line(out, "sub ecx, eax\n");
              emit_line(out, "mov eax, ecx\n");
            break;
        case AST_MUL:
            emit_line(out, "imul eax, ecx\n");
            break;
        default:
            fprintf(stderr, "Unsupported binary operator: %s\n", token_type_name(op));
    }
}

void emit_unary(FILE *out, ASTNode * node) {
    switch (node->type) {
        case AST_UNARY_NEGATE:
            emit_tree_node(out, node->unary.operand);
            emit_line(out, "neg eax\n");
            break;
        case AST_UNARY_PLUS:
            // noop
            break;
        case AST_UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_tree_node(out, node->unary.operand);
            emit_line(out, "cmp eax, 0\n");
            emit_line(out, "sete al\n");
            emit_line(out, "movzx eax, al\n");
            break;
        case AST_UNARY_PRE_INC: {
            int offset = node->unary.operand->var_expr.offset;
            emit_line(out, "mov eax, [rbp%+d]\n", offset);            
            emit_line(out, "add eax, 1\n");
            emit_line(out, "mov [rbp%+d], eax\n", offset);
            break;
        }
        case AST_UNARY_PRE_DEC: {
            int offset = node->unary.operand->var_expr.offset;
            emit_line(out, "mov eax, [rbp%+d]\n", offset);            
            emit_line(out, "sub eax, 1\n");
            emit_line(out, "mov [rbp%+d], eax\n", offset);
        break;
        }
        case AST_UNARY_POST_INC: {
            int offset = node->unary.operand->var_expr.offset;
            emit_line(out, "mov eax, [rbp%+d]\n", offset);
            emit_line(out, "mov ecx, eax\n");
            emit_line(out, "add eax, 1\n");
            emit_line(out, "mov [rbp%+d], eax\n", offset);
            emit_line(out, "mov eax, ecx\n");
            break;
        }
        case AST_UNARY_POST_DEC: {
//            int offset = lookup_symbol(node->unary.operand->var_expr.name);
            int offset = node->unary.operand->var_expr.offset;
            emit_line(out, "mov eax, [rbp%+d]\n", offset);
            emit_line(out, "mov ecx, eax\n");
            emit_line(out, "sub eax, 1\n");
            emit_line(out, "mov [rbp%+d], eax\n", offset);
            emit_line(out, "mov eax, ecx\n");
            break;
        }
        default:
            fprintf(stderr, "Unsupported unary op in emitter\n");
            exit(1);
    }
}



void emit_if_statement(FILE * out, ASTNode * node) {
    int id = label_id++;
    // eval condition
    emit_tree_node(out, node->if_stmt.cond);
    // compare result with 0
    emit_line(out, "cmp eax, 0\n");

    if (node->if_stmt.else_statement) {
        emit_line(out, "je .Lelse%d\n", id);  // jump to else if false
        emit_tree_node(out, node->if_stmt.then_statement);
        emit_line(out, "jmp .Lend%d\n", id);  // jump to end over else
        emit_label(out, "else", id);
        emit_tree_node(out, node->if_stmt.else_statement);
    }
    else {
        emit_line(out, "je .Lend%d\n", id);  // skip over if false
        emit_tree_node(out, node->if_stmt.then_statement);
    }
    emit_label(out, "end", id);
}

void emit_while_statement(FILE* out, ASTNode * node) {
    int id = label_id++;
    // start label
    emit_label(out, "while_start", id);
    // eval cond
    emit_tree_node(out, node->while_stmt.cond);
    // cmp to zero
    emit_line(out, "cmp eax, 0\n");
    // jmp to end if condition not met
    emit_line(out, "je .Lwhile_end%d\n", id);
    emit_tree_node(out, node->while_stmt.body);
    // jmp to start
    emit_line(out, "jmp .Lwhile_start%d\n", id);
    emit_label(out, "while_end", id); 
}

void emit_do_while_statement(FILE * out, ASTNode * node) {
    int id = label_id++;
    emit_label(out, "do_while_start", id);
    emit_tree_node(out, node->do_while_stmt.stmt);
    emit_tree_node(out, node->do_while_stmt.expr);
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "do_while_start", id);
    emit_label(out, "do_while_end", id);
}

void emit_block(FILE * out, ASTNode * node, bool enterNewScope) {

    for (int i=0;i<node->block.count;i++) {
        emit_tree_node(out, node->block.statements[i]);
    }

}

void emit_function(FILE * out, ASTNode * node) {

    emit_text_section_header(out);

    int local_space = node->function_decl.size;

    emit_line(out, "%s:\n", node->function_decl.name);

    emit_line(out, "push rbp\n");
    emit_line(out,  "mov rbp, rsp\n");

    if (local_space > 0) {
        emit_line(out, "sub rsp, %d\n", local_space);
    }
    
    if (node->function_decl.param_list) {
        for (struct node_list * param = node->function_decl.param_list->param_list.node_list; param != NULL; param = param->next) {
            ASTNode * var_decl = param->node;
            emit_var_declaration(out, var_decl);
        }
    }

    emit_block(out, node->function_decl.body, false);

    emit_line(out, "leave\n");
    emit_line(out, "ret\n");
}

void emit_var_declaration(FILE *out, ASTNode * node) {
    if (node->var_decl.init_expr) {
        int offset = node->var_decl.offset;
        emit_tree_node(out, node->var_decl.init_expr);
        emit_line(out, "mov [rbp%+d], eax\n", offset);
    }
}

void emit_assignment(FILE * out, ASTNode* node) {
    emit_tree_node(out, node->assignment.expr);
    int offset = node->assignment.offset;
    emit_line(out, "mov [rbp%+d], eax\n", offset);
}

void emit_add_assignment(FILE *out, ASTNode * node) {
    int offset = node->assignment.offset;
    emit_line(out, "mov eax, [rbp%+d]\n", offset);
    emit_line(out, "push rax\n");
    emit_tree_node(out, node->assignment.expr);
    emit_line(out, "pop rcx\n");
    emit_line(out, "add eax, ecx\n");
    emit_line(out, "mov [rbp%+d], eax\n", offset);
}

void emit_sub_assignment(FILE *out, ASTNode * node) {
    int offset = node->assignment.offset;
    emit_tree_node(out, node->assignment.expr);
    emit_line(out, "mov ecx, eax\n");
    emit_line(out, "mov eax, [rbp%+d]\n", offset);
    emit_line(out, "sub eax, ecx\n");
    emit_line(out, "mov [rbp%+d], eax\n", offset);
}

void emit_for_statement(FILE * out, ASTNode * node) {
    int label_start = label_id++;
    int label_cond = label_id++;
    int label_end = label_id++;

    // initializer
    if (node->for_stmt.init_expr) {
        emit_tree_node(out, node->for_stmt.init_expr);
    }

    // jump to condition check
    emit_jump(out, "jmp", "cond", label_cond);

    // loop body start
    emit_label(out, "start", label_start);
    emit_block(out, node->for_stmt.body, false);

    // update expression
    if (node->for_stmt.update_expr) {
        emit_tree_node(out, node->for_stmt.update_expr);
    }

    // loop condition
    emit_label(out, "cond", label_cond);
    if (node->for_stmt.cond_expr) {
        emit_tree_node(out, node->for_stmt.cond_expr);
        emit_line(out, "cmp eax, 0\n");
        emit_jump(out, "je", "end", label_end);     // exit if false
    }

    emit_jump(out, "jmp", "start", label_start);
    
    // end label
    emit_label(out, "end", label_end);

//    exit_scope();
}

void emit_function_call(FILE * out, struct ASTNode * node) {
    // if the call has arguments
    // first get a reversed list
    // then emit each arg then push it
    struct node_list * reversed_list = NULL;
    if (node->function_call.argument_expression_list) {
        reversed_list = reverse_list(node->function_call.argument_expression_list);

        for (struct node_list * arg = reversed_list;arg != NULL; arg = arg->next) {
            emit_tree_node(out, arg->node);
            emit_line(out, "push rax\n");
        }
    }

    // call the function
    emit_line(out, "call %s\n", node->function_call.name);

    // clean up arguments
    int total_arg_size = get_node_list_count(reversed_list) * 8;
    if (total_arg_size > 0) {
        emit_line(out, "add rsp, %d\n", total_arg_size);
    }

    if (reversed_list) {
        free_node_list(reversed_list);
    }
}

void emit_switch_dispatch(FILE* out, ASTNode * stmtList) {
    ASTNode * node = stmtList;

    while(node) {
        if (node->type == AST_CASE_STMT) {
            node->case_stmt.label = make_label("case");
            emit_line(out, "mov rax, [rsp]");
            emit_line(out, "cmp rax, %d\n", node->case_stmt.constExpression->int_value);
            emit_line(out, "je %s", node->case_stmt.label);
        }
        else if (node->type == AST_DEFAULT_STMT) {
            node->default_stmt.label = make_label("default");
            emit_line(out, "jmp %s", node->default_stmt.label);
        }

        node = node->next;
    }
}

void emit_switch_bodies(FILE * out, ASTNode * stmtList) {
    ASTNode * node = stmtList;

    while(node) {
        if (node->type == AST_CASE_STMT) {
            emit_line(out, "%s\n", node->case_stmt.label);
            emit_tree_node(out, node->case_stmt.stmt);
        }
        else if (node->type == AST_DEFAULT_STMT) {
            emit_line(out, "%s:\n", node->default_stmt.label);
            emit_tree_node(out, node->default_stmt.stmt);
        }

        node = node->next;
    }
}

void emit_switch_statement(FILE * out, ASTNode * node) {
    int label_end = label_id++;

    const char * break_label = make_label_text("switch_end", label_end);

    emit_tree_node(out, node->switch_stmt.expr);
    emit_line(out, "push rax   ; save switch expression\n");

    push_switch_context(break_label);

    // first pass emit all comparisons and jumps
    emit_switch_dispatch(out, node->switch_stmt.stmt);

    // second pass emit all labels and bodies
    emit_switch_bodies(out, node->switch_stmt.stmt);

    emit_line(out, "%s\n", break_label);
    // TODO MAY NEED TO EMIT A STACK restore
    //emit_line(out, "add rsp, 8  ; restore stack\n");

    pop_switch_context();

}

void emit_case_statement(FILE *out, ASTNode * node) {
    int case_label_id = label_id++;
    const char * case_label = make_label_text("case", case_label_id);
    node->case_stmt.label = case_label;

    // load switch value back from the stack
    emit_line(out, "mov rax, [rsp] ; reload switch expr\n");
    
    // emit the jmp to the case body
    emit_line(out, "cmp rax, %d\n", node->case_stmt.constExpression->int_value);
    emit_line(out, "je %s\n", case_label);

    // emit the case body
    emit_line(out, "%s\n", node->case_stmt.label);
    emit_tree_node(out, node->case_stmt.stmt);
    // emit jump to break

    emit_jump(out, "jmp", switch_stack->break_label);

}

const char * get_break_label() {
//    if (loop_stack) return loop_stack->break_label;    TODO
    if (switch_stack) return switch_stack->break_label;
    return NULL;
}

void emit_break_statement(FILE * out, ASTNode * node) {
    const char * label = get_break_label();
    emit_line(out, "jmp %s   ; break\n", label);
}


void emit_tree_node(FILE * out, ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {
            emit_header(out);
            for (int i=0;i<node->translation_unit.count;i++) {
                emit_tree_node(out, node->translation_unit.functions[i]);
            }
            emit_trailer(out);
            break;
        }
        case AST_FUNCTION_DECL:
            emit_function(out, node);
            break;
        case AST_VAR_DECL:
            emit_var_declaration(out, node);
            break;
        case AST_ASSIGNMENT: 
            emit_assignment(out, node);
            break;
        case AST_COMPOUND_ADD_ASSIGN:
            emit_add_assignment(out, node);
            break;
        case AST_COMPOUND_SUB_ASSIGN:
            emit_sub_assignment(out, node);
            break;
        case AST_RETURN_STMT:
            emit_tree_node(out, node->return_stmt.expr);
            emit_line(out, "leave\n");
            emit_line(out, "ret\n");
            break;
        case AST_FUNCTION_CALL:
            emit_function_call(out, node);
            break;
        case AST_EXPRESSION_STMT:
            emit_tree_node(out, node->expr_stmt.expr);
            break;
        case AST_ASSERT_EXTENSION_STATEMENT:
            emit_assert_extension_statement(out, node);
            break;
        case AST_PRINT_EXTENSION_STATEMENT:
            emit_print_extension_statement(out, node);
            break;
        case AST_BLOCK:
            emit_block(out, node, true);
            break;
        case AST_IF_STMT:
            emit_if_statement(out, node);
            break;
        case AST_WHILE_STMT:
            emit_while_statement(out, node);
            break;
        case AST_DO_WHILE_STMT:
            emit_do_while_statement(out, node);
            break;
        case AST_SWITCH_STMT:
            emit_switch_statement(out, node);
            break;
        case AST_CASE_STMT:
            emit_case_statement(out, node);
            break;
        case AST_BREAK_STMT:
            emit_break_statement(out, node);
            break;
        case AST_DIV: {
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            emit_line(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            emit_line(out, "mov ecx, eax\n");                 // move denominator to ecx
            emit_line(out, "pop rax\n");                      // restore numerator to eax
            emit_line(out, "cdq\n");
            emit_line(out, "idiv ecx\n");
            break;
        }
        case AST_MOD: {
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            emit_line(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            emit_line(out, "mov ecx, eax\n");                 // move denominator to ecx
            emit_line(out, "pop rax\n");                      // restore numerator to eax
            emit_line(out, "cdq\n");
            emit_line(out, "idiv ecx\n");               // divide eax by ecx. result goes to eax, remainder to edx
            emit_line(out, "mov eax, edx\n");           // move remainer in edx to eax
            break;
        }
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            emit_line(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            emit_line(out, "pop rcx\n");                      // pop lhs to ECX
            emit_binary_op(out, node->type);        // emit proper for op
        break;

        case AST_EQUAL:
        case AST_NOT_EQUAL:
        case AST_LESS_EQUAL:
        case AST_LESS_THAN:
        case AST_GREATER_EQUAL:
        case AST_GREATER_THAN:
            emit_binary_comparison(out, node);
            break;

        case AST_UNARY_POST_INC:
        case AST_UNARY_POST_DEC:
        case AST_UNARY_PRE_INC:
        case AST_UNARY_PRE_DEC:
        case AST_UNARY_NEGATE:
        case AST_UNARY_NOT:
        case AST_UNARY_PLUS:
            emit_unary(out, node);
            break;

        case AST_LOGICAL_AND:
            emit_logical_and(out, node);
            break;

        case AST_LOGICAL_OR:
            emit_logical_or(out, node);
            break;

        case AST_INT_LITERAL:
            emit_line(out, "mov eax, %d\n", node->int_value);
            break;
        case AST_VAR_EXPR:
            int offset = node->var_expr.offset;
            emit_line(out, "mov eax, [rbp%+d]\n", offset);
            break;
        case AST_FOR_STMT:
            emit_for_statement(out, node);
            break;
        case AST_GOTO_STMT:
            emit_jump(out, "jmp", node->goto_stmt.label, 0);
            break;

        case AST_LABELED_STMT:
            emit_label(out, node->labeled_stmt.label, 0);
            emit_tree_node(out, node->labeled_stmt.stmt);
            break;

    }
}

void emit_print_int_extension_code(FILE * out) {
    int label_convert = label_id++;
    int label_done = label_id++;
    int label_buffer = label_id++;
    int label_loop = label_id++;

    emit_bss_section_header(out);
    emit_line(out, "buffer%d resb 20\n", label_buffer);

    emit_text_section_header(out);
    emit_line(out, "print_int:\n");
    emit_line(out, "; assumes integer to print is in eax\n");
    emit_line(out, "; converts and prints using syscall\n");
    emit_line(out, "mov rcx, buffer%d + 19\n", label_buffer);
    emit_line(out, "mov byte [rcx], 10\n");
    emit_line(out, "dec rcx\n");
    emit_line(out, "\n");
    emit_line(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "convert", label_convert);
    emit_line(out, "mov byte [rcx], '0'\n");
    emit_line(out, "dec rcx\n");
    emit_jump(out, "jmp", "done", label_done);
    emit_line(out, "\n");
    emit_label(out, "convert", label_convert);
    emit_line(out, "xor edx, edx\n");
    emit_line(out, "mov ebx, 10\n");
    emit_label(out, "loop", label_loop);
    emit_line(out, "xor edx, edx\n");
    emit_line(out, "div ebx\n");
    emit_line(out, "add dl, '0'\n");
    emit_line(out, "mov [rcx], dl\n");
    emit_line(out, "dec rcx\n");
    emit_line(out, "test eax, eax\n");
    emit_jump(out, "jnz", "loop", label_loop);
    emit_line(out, "\n");
    emit_label(out, "done", label_done);
    emit_line(out, "lea rsi, [rcx + 1]\n");
    emit_line(out, "mov rdx, buffer%d + 20\n", label_buffer);
    emit_line(out, "sub rdx, rsi\n");
    emit_line(out, "mov rax, 1\n");
    emit_line(out, "mov rdi, 1\n");
    emit_line(out, "syscall\n");
    emit_line(out, "ret\n");

}

void emit(ASTNode * translation_unit, const char * output_file) {
    FILE * out = fopen(output_file, "w");
    
    emit_tree_node(out, translation_unit);

    // emit referenced private functions
    if (emit_print_int_extension) {
        emit_print_int_extension_code(out);
    }

    fclose(out);
}