//
// Created by scott on 6/19/25.
//

#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symbol.h"
#include "emitter_context.h"


EmitterContext * create_emitter_context(const char * filename) {

//    init_runtime_info_list();

    EmitterContext * ctx = malloc(sizeof(EmitterContext));
    ctx->label_id = 0;
    ctx->filename = strdup(filename);
    ctx->out = fopen(ctx->filename, "w");
    ctx->emit_print_int_extension = false;
    ctx->functionExitStack = NULL;
    ctx->switch_stack = NULL;
    ctx->loop_stack = NULL;
    return ctx;
}

EmitterContext * create_emitter_context_from_fp(FILE * file) {
    EmitterContext * ctx = malloc(sizeof(EmitterContext));
    ctx->label_id = 0;
    ctx->filename = strdup("memf");
    ctx->out = file;
    ctx->emit_print_int_extension = false;
    ctx->functionExitStack = NULL;
    ctx->switch_stack = NULL;
    ctx->loop_stack = NULL;
    return ctx;
}

void emitter_finalize(EmitterContext * ctx) {
    fclose(ctx->out);
    free(ctx->filename);
    free(ctx);
}

int get_label_id(EmitterContext * ctx) {
    return ctx->label_id++;
}

void push_function_exit_context(EmitterContext * ctx, const char * exit_label) {
    FunctionExitContext * functionCtx = malloc(sizeof(FunctionExitContext));
    functionCtx->exit_label = strdup(exit_label);
    functionCtx->next = ctx->functionExitStack;
    ctx->functionExitStack = functionCtx;
}

void pop_function_exit_context(EmitterContext * ctx) {
    if (ctx->functionExitStack) {
        FunctionExitContext * old = ctx->functionExitStack;
        ctx->functionExitStack = old->next;
        free(old->exit_label);
        free(old);
    }
}

char * get_function_exit_label(EmitterContext * ctx) {
    if (ctx->functionExitStack) {
        return ctx->functionExitStack->exit_label;
    }
    return NULL;
}

void push_switch_context(EmitterContext * ctx, const char * break_label) {
    SwitchContext * switchContext = malloc(sizeof(SwitchContext));
    switchContext->break_label = strdup(break_label);
    switchContext->next = ctx->switch_stack;
    ctx->switch_stack = switchContext;
}

void pop_switch_context(EmitterContext * ctx) {
    if (ctx->switch_stack) {
        SwitchContext * old = ctx->switch_stack;
        ctx->switch_stack = old->next;
        free(old);
    }
}

const char * current_switch_break_label(EmitterContext * ctx) {
    if (ctx->switch_stack) {
        return ctx->switch_stack->break_label;
    }
    return NULL;  // error break outside switch
}

void push_loop_context(EmitterContext * ctx, const char * start_label, const char * end_label) {
    LoopContext * loopContext = malloc(sizeof(LoopContext));
    loopContext->start_label = start_label;
    loopContext->end_label = end_label;
    loopContext->next = ctx->loop_stack;
    ctx->loop_stack = loopContext;
}

void pop_loop_context(EmitterContext * ctx) {
    if (ctx->loop_stack) {
        LoopContext * old = ctx->loop_stack;
        ctx->loop_stack = old->next;
        free(old);
    }
}

// int get_offset(EmitterContext * ctx, ASTNode * node) {
//     if (node->type == AST_VAR_DECL || node->type == AST_VAR_REF) {
//         return node->symbol->info.var.offset;
//     }
//     if (node->type == AST_ARRAY_ACCESS) {
//         // this probably needs to be replaced with code that emits this stuff
//         // and calculates at run time.
//         return node->array_access.base->symbol->info.var.offset +
//             node->array_access.index->ctype->size * node->array_access.index->int_value;
//     }
//     return 0;
// }

/* return the variable offset. for arrays's will return the base offset.
 */

int get_offset(EmitterContext * ctx, ASTNode * node) {
   if (node->type == AST_VAR_DECL || node->type == AST_VAR_REF_EXPR) {
       return node->symbol->info.var.offset;
   }
   if (node->type == AST_ARRAY_ACCESS) {
       return get_offset(ctx, node->array_access.base);
//       return node->array_access.base->symbol->info.var.offset;
   }
   return 0;
}

bool is_global_var(EmitterContext * ctx, ASTNode * node) {
    Symbol * symbol = node->type == AST_ARRAY_ACCESS ?
        node->array_access.base->symbol : node->symbol;
    return symbol->node->var_decl.is_global;
}

char * get_var_name(EmitterContext * ctx, ASTNode * node) {
    Symbol * symbol = node->type == AST_ARRAY_ACCESS ?
        node->array_access.base->symbol : node->symbol;
    return symbol->name;
}