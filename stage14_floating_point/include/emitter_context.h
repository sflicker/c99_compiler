//
// Created by scott on 6/19/25.
//

#ifndef EMITTER_CONTEXT_H
#define EMITTER_CONTEXT_H
#include <stdio.h>
#include <stdbool.h>

#include "ast.h"

typedef struct FunctionExitContext {
    char * exit_label;
    struct FunctionExitContext * next;
} FunctionExitContext;

typedef struct SwitchContext {
    const char * break_label;
    struct SwitchContext * next;
} SwitchContext;

typedef struct LoopContext {
    const char * start_label;
    const char * end_label;
    struct LoopContext * next;
} LoopContext;

typedef struct EmitterContext {
    int label_id;
    char* filename;
    FILE * out;
    bool emit_print_int_extension;
    FunctionExitContext * functionExitStack;
    SwitchContext * switch_stack;
    LoopContext * loop_stack;
    int stack_depth;
    int local_space;
} EmitterContext;

EmitterContext * create_emitter_context();
EmitterContext * create_emitter_context_from_fp(FILE * file);
int get_label_id(EmitterContext * ctx);
void emitter_finalize(EmitterContext * ctx);

void push_function_exit_context(EmitterContext * ctx, const char * exit_label);
void pop_function_exit_context(EmitterContext * ctx);
char * get_function_exit_label(EmitterContext * ctx);

void push_switch_context(EmitterContext * ctx, const char * break_label);
void pop_switch_context(EmitterContext * ctx);
const char * current_switch_break_label(EmitterContext * ctx);

void push_loop_context(EmitterContext * ctx, const char * start_label, const char * end_label);
void pop_loop_context(EmitterContext * ctx);

//int get_offset(EmitterContext * ctx, ASTNode * node);
bool is_global_var(EmitterContext * ctx, ASTNode * node);
char * get_var_name(EmitterContext * ctx, ASTNode * node);
#endif //EMITTER_CONTEXT_H
