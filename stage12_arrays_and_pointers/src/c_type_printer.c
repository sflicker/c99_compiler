//
// Created by scott on 6/29/25.
//

#include "util.h"
#include "c_type.h"

void print_c_type(CType * ctype, int indent) {
    if (!ctype) return;

    print_indent(indent);
    char buf[128];
    ctype_to_description(ctype, buf, sizeof buf);
    printf("CType: %s, size: %d, signed: %s, rank: %d\n", buf, ctype->size,
        ((ctype->is_signed) ? "TRUE" : "FALSE"), ctype->rank);
    if (ctype->base_type) {
        print_c_type(ctype->base_type, indent+1);
    }
}