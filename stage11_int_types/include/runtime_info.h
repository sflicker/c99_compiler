//
// Created by scott on 6/18/25.
//

#ifndef RUNTIME_INFO_H
#define RUNTIME_INFO_H

#include "ast.h"
#include "list.util.h"
typedef runtime_info {
    ASTNode * node;
    // TODO MORE STUFF LIKE OFFSETS AND SPECIAL LABELS
} RuntimeInfo;

DEFINE_LINKED_LIST(RuntimeInfo*, RuntimeInfo_list);

#endif //RUNTIME_INFO_H
