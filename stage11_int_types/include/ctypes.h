#ifndef __CTYPE_H
#define __CTYPE_H

#include <stdbool.h>

#include "list_util.h"

#define RANK_CHAR 1
#define RANK_SHORT 2
#define RANK_INT 3
#define RANK_LONG 4

typedef struct CType CType;

typedef enum {
    CTYPE_CHAR,
    CTYPE_SHORT,
    CTYPE_INT,
    CTYPE_LONG,
    CTYPE_PTR
} CTypeKind;

typedef struct CType {
    CTypeKind kind;
    int size;
    int is_signed;
    int rank;
    CType * ptr_to;
} CType;

extern CType CTYPE_CHAR_T;
extern CType CTYPE_SHORT_T;
extern CType CTYPE_INT_T;
extern CType CTYPE_LONG_T;
extern CType CTYPE_PTR_INT_T;



int sizeof_ctype(CType * ctype);
char * ctype_to_string(CType * ctype);

DEFINE_LINKED_LIST(CType*, CTypePtr_list);

void free_ctype(CType * ctype);

//bool ctype_equal(const CType *a, const CType *b);
CType *make_ptr_type(CType *base);

bool ctype_equals(CType * a, CType * b);
bool ctype_equal_or_compatible(CType * a, CType * b);
bool ctype_lists_equal(CTypePtr_list * a, CTypePtr_list * b);

CType * common_type(CType *a, CType *b);
CTypePtr_list * astNodeListToTypeList(const ASTNode_list * param_list);

#endif

