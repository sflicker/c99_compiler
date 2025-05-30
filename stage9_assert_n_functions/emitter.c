#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "emitter.h"
#include "token.h"
#include "util.h"

void emit_tree_node(FILE * out, ASTNode * node);
void emit_var_declaration(FILE *out, ASTNode * node);

bool emit_print_int_extension = false;

static int label_id = 0;

void emit_header(FILE* out) {
    fprintf(out, "section .text\n");
    fprintf(out, "global main\n");
    fprintf(out, "\n");
}

void emit_trailer(FILE* out) {
    fprintf(out, "\n");
    fprintf(out, "section .rodata\n");
    fprintf(out, "assert_fail_msg: db \"Assertion failed!\", 10\n");
}

void emit_text_section_header(FILE * out) {
    fprintf(out, "\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, ";   SECTION: Text (Code)\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, "\n");
    fprintf(out, "section .text\n");
    fprintf(out, "\n");
}

void emit_data_section_header(FILE * out) {
    fprintf(out, "\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, ";   SECTION: Data (Initialized globals/strings\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, "\n");
    fprintf(out, "section .data\n");
    fprintf(out, "\n");
}

void emit_bss_section_header(FILE * out) {
    fprintf(out, "\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, ";   SECTION: BSS (Uninitialized buffers)\n");
    fprintf(out, ";---------------------------------------\n");
    fprintf(out, "\n");
    fprintf(out, "section .bss\n");
    fprintf(out, "\n");
}

void emit_label(FILE * out, const char * prefix, int num) {
    fprintf(out, ".L%s%d:\n", prefix, num);
}

void emit_jump(FILE * out, const char * op, const char * prefix, int num) {
    fprintf(out, "%s .L%s%d\n", op, prefix, num);
}

void emit_assert_extension_statement(FILE * out, ASTNode * node) {
    int label_pass = label_id++;

    // evaluate expression
    emit_tree_node(out, node->expr_stmt.expr);

    // compare result in eax with 0
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "assert_pass", label_pass);

    // assert failed
    // print message
    fprintf(out, "mov rax, 1\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "lea rsi, [rel assert_fail_msg]\n");
    fprintf(out, "mov rdx, 17\n");
    fprintf(out, "syscall\n");

    // exit
    fprintf(out, "mov rax, 60\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "syscall\n");

    emit_label(out, "assert_pass", label_pass);

}

void emit_print_extension_statement(FILE * out, ASTNode * node) {
    // emit the expression storing it in EAX
    emit_tree_node(out, node->expr_stmt.expr);
    fprintf(out, "call print_int\n");
    emit_print_int_extension = true;
}

void emit_logical_and(FILE * out, ASTNode * node) {
    int label_false = label_id++;
    int label_end = label_id++;

    //lhs 
    emit_tree_node(out, node->binary.lhs);
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "je", "false", label_false);

    //rhs
    emit_tree_node(out, node->binary.rhs);
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "je", "false", label_false);

    // both true
    fprintf(out, "mov eax, 1\n");
    emit_jump(out, "jmp", "end", label_end);

    emit_label(out, "false", label_false);
    fprintf(out, "mov eax, 0\n");

    emit_label(out, "end", label_end);
}

void emit_logical_or(FILE* out, ASTNode* node) {
    int label_true = label_id++;
    int label_end = label_id++;

    // lhs
    emit_tree_node(out, node->binary.lhs);
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "true", label_true);

    // rhs
    emit_tree_node(out, node->binary.rhs);
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "true", label_true);

    emit_label(out, "true", label_true);
    fprintf(out, "mov eax, 1\n");

    emit_label(out, "end", label_end);
}


void emit_binary_comparison(FILE * out, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_tree_node(out, node->binary.lhs);
    fprintf(out, "push rax\n");

    // eval right-hand side -> reult in eax

    emit_tree_node(out, node->binary.rhs);

    // restore lhs into rcx
    fprintf(out, "pop rcx\n");
    fprintf(out, "mov ecx, ecx\n");   // zero upper bits

    // compare rcx (lhs) with eax (rhs), cmp rcx, eax means rcx - eax
    fprintf(out, "cmp ecx, eax\n");

    // emit proper setX based on operator type
    switch (node->type) {
        case AST_EQUAL:
            fprintf(out, "sete al\n");
            break;

        case AST_NOT_EQUAL:
            fprintf(out, "setne al\n");
            break;

        case AST_LESS_THAN:
            fprintf(out, "setl al\n");
            break;

        case AST_LESS_EQUAL:
            fprintf(out, "setle al\n");
            break;

        case AST_GREATER_THAN:
            fprintf(out, "setg al\n");
            break;

        case AST_GREATER_EQUAL:
            fprintf(out, "setge al\n");
            break;

        default:
            fprintf(stderr, "Unsupported comparison type in codegen.\n");
            exit(1);

    }

    // zero-extend result to full eax
    fprintf(out, "movzx eax, al\n");

}


void emit_binary_op(FILE * out, ASTNodeType op) {
    switch(op) {
        case AST_ADD:
            fprintf(out, "add eax, ecx\n");
            break;
        case AST_SUB:
//            fprintf(out, "sub eax, ecx\n");
              fprintf(out, "sub ecx, eax\n");
              fprintf(out, "mov eax, ecx\n");
            break;
        case AST_MUL:
            fprintf(out, "imul eax, ecx\n");
            break;
        default:
            fprintf(stderr, "Unsupported binary operator: %s\n", token_type_name(op));
    }
}

void emit_unary(FILE *out, ASTNode * node) {
    switch (node->type) {
        case AST_UNARY_NEGATE:
            emit_tree_node(out, node->unary.operand);
            fprintf(out, "neg eax\n");
            break;
        case AST_UNARY_PLUS:
            // noop
            break;
        case AST_UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_tree_node(out, node->unary.operand);
            fprintf(out, "cmp eax, 0\n");
            fprintf(out, "sete al\n");
            fprintf(out, "movzx eax, al\n");
            break;
        case AST_UNARY_PRE_INC: {
            int offset = node->unary.operand->var_expr.offset;
            fprintf(out, "mov eax, [rbp%+d]\n", offset);            
            fprintf(out, "add eax, 1\n");
            fprintf(out, "mov [rbp%+d], eax\n", offset);
            break;
        }
        case AST_UNARY_PRE_DEC: {
            int offset = node->unary.operand->var_expr.offset;
            fprintf(out, "mov eax, [rbp%+d]\n", offset);            
            fprintf(out, "sub eax, 1\n");
            fprintf(out, "mov [rbp%+d], eax\n", offset);
        break;
        }
        case AST_UNARY_POST_INC: {
            int offset = node->unary.operand->var_expr.offset;
            fprintf(out, "mov eax, [rbp%+d]\n", offset);
            fprintf(out, "mov ecx, eax\n");
            fprintf(out, "add eax, 1\n");
            fprintf(out, "mov [rbp%+d], eax\n", offset);
            fprintf(out, "mov eax, ecx\n");
            break;
        }
        case AST_UNARY_POST_DEC: {
//            int offset = lookup_symbol(node->unary.operand->var_expr.name);
            int offset = node->unary.operand->var_expr.offset;
            fprintf(out, "mov eax, [rbp%+d]\n", offset);
            fprintf(out, "mov ecx, eax\n");
            fprintf(out, "sub eax, 1\n");
            fprintf(out, "mov [rbp%+d], eax\n", offset);
            fprintf(out, "mov eax, ecx\n");
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
    fprintf(out, "cmp eax, 0\n");

    if (node->if_stmt.else_statement) {
        fprintf(out, "je .Lelse%d\n", id);  // jump to else if false
        emit_tree_node(out, node->if_stmt.then_statement);
        fprintf(out, "jmp .Lend%d\n", id);  // jump to end over else
        emit_label(out, "else", id);
        emit_tree_node(out, node->if_stmt.else_statement);
    }
    else {
        fprintf(out, "je .Lend%d\n", id);  // skip over if false
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
    fprintf(out, "cmp eax, 0\n");
    // jmp to end if condition not met
    fprintf(out, "je .Lwhile_end%d\n", id);
    emit_tree_node(out, node->while_stmt.body);
    // jmp to start
    fprintf(out, "jmp .Lwhile_start%d\n", id);
    emit_label(out, "while_end", id); 
}


void emit_block(FILE * out, ASTNode * node, bool enterNewScope) {

    for (int i=0;i<node->block.count;i++) {
        emit_tree_node(out, node->block.statements[i]);
    }

}

void emit_function(FILE * out, ASTNode * node) {

    emit_text_section_header(out);

    int local_space = node->function_decl.size;

    fprintf(out, "%s:\n", node->function_decl.name);

    fprintf(out, "push rbp\n");
    fprintf(out,  "mov rbp, rsp\n");

    if (local_space > 0) {
        fprintf(out, "sub rsp, %d\n", local_space);
    }
    
    if (node->function_decl.param_list) {
        for (struct node_list * param = node->function_decl.param_list->param_list.node_list; param != NULL; param = param->next) {
            ASTNode * var_decl = param->node;
            emit_var_declaration(out, var_decl);
        }
    }

    emit_block(out, node->function_decl.body, false);

    fprintf(out, "leave\n");
    fprintf(out, "ret\n");
}

void emit_var_declaration(FILE *out, ASTNode * node) {
    if (node->var_decl.init_expr) {
        int offset = node->var_decl.offset;
        emit_tree_node(out, node->var_decl.init_expr);
        fprintf(out, "mov [rbp%+d], eax\n", offset);
    }
}

void emit_assignment(FILE * out, ASTNode* node) {
    emit_tree_node(out, node->assignment.expr);
    int offset = node->assignment.offset;
    fprintf(out, "mov [rbp%+d], eax\n", offset);
}

void emit_add_assignment(FILE *out, ASTNode * node) {
    int offset = node->assignment.offset;
    fprintf(out, "mov eax, [rbp%+d]\n", offset);
    fprintf(out, "push rax\n");
    emit_tree_node(out, node->assignment.expr);
    fprintf(out, "pop rcx\n");
    fprintf(out, "add eax, ecx\n");
    fprintf(out, "mov [rbp%+d], eax\n", offset);
}

void emit_sub_assignment(FILE *out, ASTNode * node) {
    int offset = node->assignment.offset;
    emit_tree_node(out, node->assignment.expr);
    fprintf(out, "mov ecx, eax\n");
    fprintf(out, "mov eax, [rbp%+d]\n", offset);
    fprintf(out, "sub eax, ecx\n");
    fprintf(out, "mov [rbp%+d], eax\n", offset);
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
        fprintf(out, "cmp eax, 0\n");
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
            fprintf(out, "push rax\n");
        }
    }

    // call the function
    fprintf(out, "call %s\n", node->function_call.name);

    // clean up arguments
    int total_arg_size = get_node_list_count(reversed_list) * 8;
    if (total_arg_size > 0) {
        fprintf(out, "add rsp, %d\n", total_arg_size);
    }

    if (reversed_list) {
        free_node_list(reversed_list);
    }
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
            fprintf(out, "leave\n");
            fprintf(out, "ret\n");
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
        case AST_DIV: {
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            fprintf(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            fprintf(out, "mov ecx, eax\n");                 // move denominator to ecx
            fprintf(out, "pop rax\n");                      // restore numerator to eax
            fprintf(out, "cdq\n");
            fprintf(out, "idiv ecx\n");
            break;
        }
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            fprintf(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            fprintf(out, "pop rcx\n");                      // pop lhs to ECX
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
            fprintf(out, "mov eax, %d\n", node->int_value);
            break;
        case AST_VAR_EXPR:
            int offset = node->var_expr.offset;
            fprintf(out, "mov eax, [rbp%+d]\n", offset);
            break;
        case AST_FOR_STMT:
            emit_for_statement(out, node);
            break;

    }
}

void emit_print_int_extension_code(FILE * out) {
    int label_convert = label_id++;
    int label_done = label_id++;
    int label_buffer = label_id++;
    int label_loop = label_id++;

    emit_bss_section_header(out);
    fprintf(out, "buffer%d resb 20\n", label_buffer);

    emit_text_section_header(out);
    fprintf(out, "print_int:\n");
    fprintf(out, "; assumes integer to print is in eax\n");
    fprintf(out, "; converts and prints using syscall\n");
    fprintf(out, "mov rcx, buffer%d + 19\n", label_buffer);
    fprintf(out, "mov byte [rcx], 10\n");
    fprintf(out, "dec rcx\n");
    fprintf(out, "\n");
    fprintf(out, "cmp eax, 0\n");
    emit_jump(out, "jne", "convert", label_convert);
    fprintf(out, "mov byte [rcx], '0'\n");
    fprintf(out, "dec rcx\n");
    emit_jump(out, "jmp", "done", label_done);
    fprintf(out, "\n");
    emit_label(out, "convert", label_convert);
    fprintf(out, "xor edx, edx\n");
    fprintf(out, "mov ebx, 10\n");
    emit_label(out, "loop", label_loop);
    fprintf(out, "xor edx, edx\n");
    fprintf(out, "div ebx\n");
    fprintf(out, "add dl, '0'\n");
    fprintf(out, "mov [rcx], dl\n");
    fprintf(out, "dec rcx\n");
    fprintf(out, "test eax, eax\n");
    emit_jump(out, "jnz", "loop", label_loop);
    fprintf(out, "\n");
    emit_label(out, "done", label_done);
    fprintf(out, "lea rsi, [rcx + 1]\n");
    fprintf(out, "mov rdx, buffer%d + 20\n", label_buffer);
    fprintf(out, "sub rdx, rsi\n");
    fprintf(out, "mov rax, 1\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "syscall\n");
    fprintf(out, "ret\n");

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