//
// Created by scott on 6/16/25.
//

#include <stdbool.h>

#include "analyzer_context.h"

#include <stdlib.h>



AnalyzerContext * analyzer_context_new() {
    AnalyzerContext * context = (AnalyzerContext *)malloc(sizeof(AnalyzerContext));
    context->current_function_return_type = NULL;
    return context;
}

void analyzer_context_free(AnalyzerContext * context) {
    free(context);
}

void setTranslationUnit(AnalyzerContext * context, ASTNode * translation_unit) {
    context->translation_unit = translation_unit;
}

ASTNode * getTranslationUnit(AnalyzerContext * context) {
    return context->translation_unit;
}