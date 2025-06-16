//
// Created by scott on 6/16/25.
//

#include <stdbool.h>

#include "analyzer_context.h"

#include <stdlib.h>

AnalyzerContext * analyzer_context_new() {
    AnalyzerContext * context = (AnalyzerContext *)malloc(sizeof(AnalyzerContext));
    context->current_function_return_type = NULL;
    context->make_new_scope = false;
    return context;
}
