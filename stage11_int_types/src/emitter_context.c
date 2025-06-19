//
// Created by scott on 6/19/25.
//

#include <stdlib.h>
#include <string.h>

#include "emitter_context.h"


EmitterContext * create_emitter_context(const char * filename) {
    EmitterContext * ctx = malloc(sizeof(EmitterContext));
    ctx->label_id = 0;
    ctx->filename = strdup(filename);
    ctx->out = fopen(ctx->filename, "w");
    ctx->emit_print_int_extension = false;
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