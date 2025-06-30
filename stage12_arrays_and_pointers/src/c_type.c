#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
#include "c_type.h"

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
    return t;
}
CType *make_pointer_type(CType *base) {
    CType *ptr = make_type();
    ptr->kind = CTYPE_PTR;
    ptr->base_type = base;
    ptr->size = 8;
    return ptr;
}

CType * make_array_type(CType * base, int length) {
    CType * ptr = make_type();
    ptr->kind = CTYPE_ARRAY;
    ptr->array_len = length;
    ptr->base_type = base;
    ptr->size = base->size * length;
    return ptr;
}

CType * make_function_type(CType * return_type, CType_list * param_types) {
    CType * fn = make_type();
    fn->kind = CTYPE_FUNCTION;
    fn->base_type = return_type;
    fn->param_types = param_types;
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

char * ctype_to_string(CType * ctype) {

    if (!ctype) return strdup("<null>");

    switch(ctype->kind) {
        case CTYPE_CHAR: return strdup("char");
        case CTYPE_SHORT: return "short";
        case CTYPE_INT: return "int";
        case CTYPE_LONG: return "long";
        case CTYPE_PTR: {
            char * inner = ctype_to_string(ctype->base_type);
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

bool type_is_compatible(CType * expected, CType * actual) {
    if (expected == NULL || actual == NULL) return false;
    if (expected->kind == actual->kind) return true;   // TODO need to add check for unsigned
    if (is_integer_type(expected) && is_integer_type(actual)) {
        if (actual->rank <= expected->rank) {
            return true;
        }
    }
    return false;
}

bool ctype_equal_or_compatible(CType * a, CType * b) {
    return ctype_equals(a, b) || type_is_compatible(a, b);
}

bool ctype_lists_equal(CTypePtr_list * a, CTypePtr_list * b) {
    if (a == b) return true;
    if (!a || !b) return false;

    if (a->count != b->count) {
        return false;
    }

    CTypePtr_list_node * a_node = a->head;
    CTypePtr_list_node * b_node = b->head;
    while (a_node && b_node) {
        if (!ctype_equals(a_node->value, b_node->value)) {
            return false;
        }
        a_node = a_node->next;
        b_node = b_node->next;
    }
    return true;
}

CType * common_type(CType *a, CType *b) {
    if (a->kind == CTYPE_LONG && b->kind == CTYPE_LONG) return &CTYPE_LONG_T;
    if (a->kind == CTYPE_INT && b->kind == CTYPE_INT) return &CTYPE_INT_T;
    if (a->kind == CTYPE_SHORT && b->kind == CTYPE_SHORT) return &CTYPE_INT_T;  // promote to int
    if (a->kind == CTYPE_CHAR && b->kind == CTYPE_CHAR) return &CTYPE_INT_T;  // promote to int
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
