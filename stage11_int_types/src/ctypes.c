#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctypes.h"


int sizeof_ctype(CType * ctype) {
    return ctype->size;
}

char * ctype_to_string(CType * ctype) {

    if (!ctype) return strdup("<null>");

    switch(ctype->kind) {
        case CTYPE_CHAR: return strdup("char");
        case CTYPE_SHORT: return "short";
        case CTYPE_INT: return "int";
        case CTYPE_LONG: return "long";
        case CTYPE_PTR: {
            char * inner = ctype_to_string(ctype->ptr_to);
            size_t len = strlen(inner) + 2;
            char * out = malloc(len + 1);
            snprintf(out, len + 1, "%s*", inner);
            free(inner);
            return out;
        }
        default:
            return strdup("<unknown>");
    }
}

CType CTYPE_CHAR_T = {CTYPE_CHAR, 1, 1, NULL};
CType CTYPE_SHORT_T = {CTYPE_SHORT, 2, 1, NULL};
CType CTYPE_INT_T = {CTYPE_INT, 4, 1, NULL};
CType CTYPE_LONG_T = {CTYPE_LONG, 8, 1, NULL};
CType CTYPE_PTR_INT_T = {CTYPE_PTR, 8, 1, &CTYPE_INT_T};

void free_ctype(CType * ctype) {
    // do nothing
}

// bool ctype_equal(const CType *a, const CType *b) {
//     if (a == b) return true; // handles NULL == NULL or pointer identity
//     if (!a || !b) return false; // one is NULL, the other isn't
//
//     if (a->kind != b->kind)
//         return false;
//
//     if (a->kind == CTYPE_PTR)
//         return ctype_equal(a->ptr_to, b->ptr_to);
//
//     return true;
// }

CType *make_ptr_type(CType *base) {
    CType *ptr = malloc(sizeof(CType));
    ptr->kind = CTYPE_PTR;
    ptr->ptr_to = base;
    return ptr;
}

bool ctype_equals(CType * a, CType * b) {
    if (a == b) return true;
    if (!a || !b) return a == b;

    if ( a->kind == b->kind
        && a->size == b->size
        && a->is_signed == b->is_signed) {
        return true;
    }

    if (a->kind == CTYPE_PTR)
        return ctype_equals(a->ptr_to, b->ptr_to);

    return false;
}