#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "c_type.h"
#include "c_type_printer.h"
#include "error.h"

CType CTYPE_CHAR_T = {
    .kind = CTYPE_CHAR,
    .base_type = NULL,
    .size = 1,
    .is_signed = 1,
    .rank = RANK_CHAR,
    .array_len = 0
};

CType CTYPE_SHORT_T = {
    .kind = CTYPE_SHORT,
    .base_type = NULL,
    .size = 2,
    .is_signed = 1,
    .rank = RANK_SHORT,
    .array_len = 0
};

CType CTYPE_INT_T = {
    .kind = CTYPE_INT,
    .base_type = NULL,
    .size = 4,
    .is_signed = 1,
    .rank = RANK_INT,
    .array_len = 0
};

CType CTYPE_LONG_T = {
    .kind = CTYPE_LONG,
    .base_type = NULL,
    .size = 8,
    .is_signed = 1,
    .rank = RANK_LONG,
    .array_len = 0
};

CType * make_type() {
    return calloc(1, sizeof(CType));
}

CType * make_int_type(int is_signed) {
    CType * t = make_type();
    t->is_signed = is_signed;
    t->kind = CTYPE_INT;
    t->size = 4;
    print_c_type(t, 0);
    return t;
}

CType * make_char_type(bool is_signed) {
    CType * t = make_type();
    t->is_signed = is_signed;
    t->kind = CTYPE_CHAR;
    t->size = 1;
    print_c_type(t, 0);
    return t;
}
CType *make_pointer_type(CType *base) {
    CType *ptr = make_type();
    ptr->kind = CTYPE_PTR;
    ptr->base_type = base;
    ptr->size = 8;
    ptr->array_len = 0;
    print_c_type(ptr, 0);
    return ptr;
}

CType * make_array_type(CType * base, int length) {
    CType * ptr = make_type();
    ptr->kind = CTYPE_ARRAY;
    ptr->array_len = length;
    ptr->base_type = base;
    ptr->size = base->size * length;
    print_c_type(ptr, 0);
    return ptr;
}

CType * make_function_type(CType * return_type, CType_list * param_types) {
    CType * fn = make_type();
    fn->kind = CTYPE_FUNCTION;
    fn->base_type = return_type;
    fn->param_types = param_types;
    print_c_type(fn, 0);
    return fn;
}

CType * copy_type(const CType * src) {
    CType * copy = make_type();
    *copy = *src;
    return copy;
}

int sizeof_ctype(CType * ctype) {
    return ctype->size;
}

void ctype_to_description(CType * ctype, char * buf, size_t buflen) {
    if (!ctype) {
        snprintf(buf, buflen, "<null>");
        return;
    }
    char inner[128];
    inner[0] = '\0';

    switch(ctype->kind) {
        case CTYPE_CHAR:
            strncat(buf, "char", buflen - strlen(buf) - 1);
            break;
        case CTYPE_SHORT:
            strncat(buf, "short", buflen - strlen(buf) - 1);
            break;
        case CTYPE_INT:
            strncat(buf, "int", buflen - strlen(buf) - 1);
            break;
        case CTYPE_LONG:
            strncat(buf, "long", buflen - strlen(buf) - 1);
            break;
        case CTYPE_PTR: {
            ctype_to_description(ctype->base_type, inner, 128);

            snprintf(buf, buflen, "Pointer to %s", inner);
//            strncat(buf, tmp, buflen - sizeof(buf) - 1);
            break;
        }
        case CTYPE_ARRAY: {
            ctype_to_description(ctype->base_type, inner, 128);
            snprintf(buf, buflen, "Array [%d] of %s", ctype->array_len, inner);
//            strncat(buf, tmp, buflen - sizeof(buf) - 1);
            break;
        }
        case CTYPE_FUNCTION: {
            strncat(buf, "function returning ", buflen - strlen(buf) - 1);
            ctype_to_description(ctype->base_type, buf, buflen);
            break;
        }
        default:
            strncat(buf, "<unknown>", buflen - sizeof(buf) - 1);
            break;
    }
}

void ctype_to_cdecl(CType * ctype, char * buf, size_t buflen) {

    if (!ctype) {
        snprintf(buf, buflen, "<null>");
        return;
    }

    switch(ctype->kind) {
        case CTYPE_CHAR:
            strncat(buf, "char", buflen - strlen(buf) - 1);
            break;
        case CTYPE_SHORT:
            strncat(buf, "short", buflen - strlen(buf) - 1);
            break;
        case CTYPE_INT:
            strncat(buf, "int", buflen - strlen(buf) - 1);
            break;
        case CTYPE_LONG:
            strncat(buf, "long", buflen - strlen(buf) - 1);
            break;
        case CTYPE_PTR: {
            char inner[64];
            inner[0] = '\0';
            ctype_to_cdecl(ctype->base_type, inner, sizeof(inner));
//            size_t len = strlen(inner) + 2;
            snprintf(buf, buflen - strlen(buf) - 1, "%s*", inner);
            break;
        }
        case CTYPE_ARRAY: {
            ctype_to_cdecl(ctype->base_type, buf, buflen);
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "[%d]", ctype->array_len);
            strncat(buf, tmp, buflen - sizeof(buf) - 1);
            break;
        }
        default:
            strncat(buf, "<unknown>", buflen - strlen(buf) - 1);
    }
}


void free_ctype(CType * ctype) {
    // do nothing
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
        return ctype_equals(a->base_type, b->base_type);

    return false;
}

bool is_integer_type(CType * ctype) {
    return ctype->kind == CTYPE_INT ||
            ctype->kind == CTYPE_LONG ||
            ctype->kind == CTYPE_SHORT ||
            ctype->kind == CTYPE_CHAR;
}

bool is_array_type(CType * ctype) {
    return ctype->kind == CTYPE_ARRAY;
}

bool is_function_type(CType * ctype) {
    return ctype->kind == CTYPE_FUNCTION;
}

bool is_pointer_type(CType * ctype) {
    return ctype->kind == CTYPE_PTR;
}

bool is_signed_type(CType * ctype) {
    return ctype->is_signed;
}

CType * decay_if_array(CType * ctype) {
    if (ctype->kind == CTYPE_ARRAY) {
        return make_pointer_type(ctype->base_type);
    }
    return ctype;
}

bool type_is_compatible(CType * lhs, CType * rhs) {
    if (lhs == NULL || rhs == NULL) return false;
    if (lhs->kind == rhs->kind) return true;   // TODO need to add check for unsigned
    if (is_integer_type(lhs) && is_integer_type(rhs)) {
        if (rhs->rank <= lhs->rank) {
            return true;
        }
    }
    return false;
}

bool ctype_equal_or_compatible(CType * lhs, CType * rhs) {
    return ctype_equals(lhs, rhs) || type_is_compatible(lhs, rhs);
}

bool ctype_lists_equal(CTypePtr_list * lhs, CTypePtr_list * rhs) {
    if (lhs == rhs) return true;
    if (!lhs || !rhs) return false;

    if (lhs->count != rhs->count) {
        return false;
    }

    CTypePtr_list_node * lhs_node = lhs->head;
    CTypePtr_list_node * rhs_node = rhs->head;
    while (lhs_node && rhs_node) {
        if (!ctype_equals(lhs_node->value, rhs_node->value)) {
            return false;
        }
        lhs_node = lhs_node->next;
        rhs_node = rhs_node->next;
    }
    return true;
}

CType * common_type(CType *lhs, CType *rhs) {
    if (lhs->kind == CTYPE_LONG && rhs->kind == CTYPE_LONG) return &CTYPE_LONG_T;
    if (lhs->kind == CTYPE_INT && rhs->kind == CTYPE_INT) return &CTYPE_INT_T;
    if (lhs->kind == CTYPE_SHORT && rhs->kind == CTYPE_SHORT) return &CTYPE_INT_T;  // promote to int
    if (lhs->kind == CTYPE_CHAR && rhs->kind == CTYPE_CHAR) return &CTYPE_INT_T;  // promote to int
    return NULL;
}

// paramList should be made of a list of VarDecl
CTypePtr_list * astNodeListToTypeList(const ASTNode_list * param_list) {
    if (param_list == NULL) return NULL;

    CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(typeList, free_ctype);
    for (ASTNode_list_node * n = param_list->head; n != NULL; n = n->next) {
        CTypePtr_list_append(typeList, n->value->ctype);
    }
    return typeList;
}

char * c_type_kind_to_string(CTypeKind kind) {
    switch (kind) {
        case CTYPE_CHAR: return "CHAR_T";
        case CTYPE_SHORT: return "SHORT_T";
        case CTYPE_INT: return "INT_T";
        case CTYPE_LONG: return "LONG_T";
        case CTYPE_PTR: return "PTR_T";
        case CTYPE_ARRAY: return "ARRAY_T";
        case CTYPE_FUNCTION: return "FUNCTION_T";
            default: return "<unknown>";
    }
}

int sizeof_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR:     return 1;
        case CTYPE_SHORT:    return 2;
        case CTYPE_INT:      return 4;
        case CTYPE_LONG:     return 8;
        case CTYPE_PTR:      return 8;
        case CTYPE_ARRAY:    return ctype->array_len * sizeof_type(ctype->base_type);
        case CTYPE_FUNCTION: error("Cannot apply sizeof to function type"); return 0;
        default: error("Unknown type"); return 0;
    }
}

int sizeof_basetype(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR:     return 1;
        case CTYPE_SHORT:    return 2;
        case CTYPE_INT:      return 4;
        case CTYPE_LONG:     return 8;
        case CTYPE_PTR:      return 8;
        case CTYPE_ARRAY:    return sizeof_basetype(ctype->base_type);
        case CTYPE_FUNCTION: error("Cannot apply sizeof to function type"); return 0;
        default: error("Unknown type"); return 0;
    }
}

