//
// Created by scott on 6/14/25.
//

#ifndef ANALYZER_H
#define ANALYZER_H
#include <stdbool.h>

#include "ast.h"
void analyze(ASTNode * node, bool make_new_scope);

#endif //ANALYZER_H
