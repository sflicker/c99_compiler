//
// Created by scott on 6/29/25.
//

#include "util.h"
#include "c_type.h"

void print_c_type(CType * ctype, int indent) {
    if (!ctype) return;

    print_indent(indent);

    printf("CType: %s, size: %d, signed: %s, rank: %d\n", c_type_kind_to_string(ctype->kind), ctype->size,
        ((ctype->is_signed) ? "TRUE" : "FALSE"), ctype->rank);
    if (ctype->base_type) {
        print_c_type(ctype->base_type, indent+1);
    }
}