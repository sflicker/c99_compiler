//
// Created by scott on 7/13/25.
//

#include "emitter.h"
#include "emitter_context.h"
#include "emitter_helpers.h"

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

void emit_assert_extension_statement(EmitterContext * ctx, ASTNode * node) {
    int label_pass = get_label_id(ctx);

    // evaluate expression
    emit_tree_node(ctx, node->expr_stmt.expr);
    emit_pop(ctx, "rax");

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
    emit_add_rsp(ctx, 8);
    ctx->emit_print_int_extension = true;
}
