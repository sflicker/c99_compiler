//
// Created by scott on 6/18/25.
//

#ifndef RUNTIME_INFO_H
#define RUNTIME_INFO_H

#include "ast.h"
#include "list_util.h"
typedef struct RuntimeInfo {
    ASTNode * node;
    int offset;
    int size;
    // TODO MORE STUFF LIKE OFFSETS AND SPECIAL LABELS
} RuntimeInfo;

void free_runtime_info();
DEFINE_LINKED_LIST(RuntimeInfo*, RuntimeInfo_list);

#endif //RUNTIME_INFO_H
