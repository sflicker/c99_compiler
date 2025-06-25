//
// Created by scott on 6/16/25.
//

#ifndef ANALYZER_CONTEXT_H
#define ANALYZER_CONTEXT_H

#include "ctypes.h"
typedef struct AnalyzerContext {
    CType * current_function_return_type;
} AnalyzerContext;

AnalyzerContext * analyzer_context_new();
void analyzer_context_free(AnalyzerContext * context);

#endif //ANALYZER_CONTEXT_H
