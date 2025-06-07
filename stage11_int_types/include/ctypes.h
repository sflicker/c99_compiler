#ifndef __CTYPE_H
#define __CTYPE_H

#include "list_util.h"

typedef enum {
    CTYPE_CHAR,
    CTYPE_SHORT,
    CTYPE_INT,
    CTYPE_LONG
} CTypeKind;

typedef struct {
    CTypeKind kind;
    int size;
    int is_signed;
} CType;

extern CType CTYPE_CHAR_T;
extern CType CTYPE_SHORT_T;
extern CType CTYPE_INT_T;
extern CType CTYPE_LONG_T;

int sizeof_ctype(CType * ctype);
const char * ctype_name(CType * ctype);

DEFINE_LINKED_LIST(CType*, CTypePtr_list);

void free_type(CType * ctype);

#endif

