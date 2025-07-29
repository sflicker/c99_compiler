//
// Created by scott on 6/16/25.
//

#ifndef ANALYZER_CONTEXT_H
#define ANALYZER_CONTEXT_H

#include "c_type.h"
typedef struct AnalyzerContext {
    CType * current_function_return_type;
    ASTNode * translation_unit;
} AnalyzerContext;

AnalyzerContext * analyzer_context_new();
void analyzer_context_free(AnalyzerContext * context);
void setTranslationUnit(AnalyzerContext * context, ASTNode * translation_unit);
ASTNode * getTranslationUnit(AnalyzerContext * context);

#endif //ANALYZER_CONTEXT_H
