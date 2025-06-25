#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast.h"
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

CType CTYPE_CHAR_T = {CTYPE_CHAR, 1, 1, RANK_CHAR};
CType CTYPE_SHORT_T = {CTYPE_SHORT, 2, 1, RANK_SHORT};
CType CTYPE_INT_T = {CTYPE_INT, 4, 1, RANK_INT};
CType CTYPE_LONG_T = {CTYPE_LONG, 8, 1, RANK_LONG};
CType CTYPE_PTR_INT_T = {CTYPE_PTR, 8, 1};

void free_ctype(CType * ctype) {
    // do nothing
}

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

bool is_integer_type(CType * ctype) {
    return ctype->kind == CTYPE_INT ||
            ctype->kind == CTYPE_LONG ||
            ctype->kind == CTYPE_SHORT ||
            ctype->kind == CTYPE_CHAR;
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
