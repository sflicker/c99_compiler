
#include "ctypes.h"

int sizeof_ctype(CType * ctype) {
    return ctype->size;
}

const char * ctype_name(CType * ctype) {
    switch(ctype->kind) {
        case CTYPE_CHAR: return "CHAR";
        case CTYPE_SHORT: return "SHORT";
        case CTYPE_INT: return "INT";
        case CTYPE_LONG: return "LONG";
    }
    return "UNKNOWN";
}

CType CTYPE_CHAR_T = {CTYPE_CHAR, 1, 1};
CType CTYPE_SHORT_T = {CTYPE_SHORT, 2, 1};
CType CTYPE_INT_T = {CTYPE_INT, 4, 1};
CType CTYPE_LONG_T = {CTYPE_LONG, 8, 1};

void free_ctype(CType * ctype) {
    // do nothing
}