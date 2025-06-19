//
// Created by scott on 6/19/25.
//

#ifndef EMITTER_CONTEXT_H
#define EMITTER_CONTEXT_H
#include <stdio.h>
#include <stdbool.h>

typedef struct EmitterContext {
    int label_id;
    char* filename;
    FILE * out;
    bool emit_print_int_extension;
} EmitterContext;

EmitterContext * create_emitter_context();
int get_label_id(EmitterContext * ctx);
void emitter_finalize(EmitterContext * ctx);

#endif //EMITTER_CONTEXT_H
