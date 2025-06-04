#include "type.h"

int sizeof_type(Type * type) {
    return type->size;
}

const char * type_name(Type * type) {
    switch(type->kind) {
        case TYPE_CHAR: return "CHAR";
        case TYPE_SHORT: return "SHORT";
        case TYPE_INT: return "INT";
        case TYPE_LONG: return "LONG";
    }
    return "UNKNOWN";
}

Type TYPE_CHAR_T = {TYPE_CHAR, 1, 1};
Type TYPE_SHORT_T = {TYPE_SHORT, 2, 1};
Type TYPE_INT_T = {TYPE_INT, 4, 1};
Type TYPE_LONG_T = {TYPE_LONG, 8, 1};

void free_type(Type * type) {
    // do nothing
}