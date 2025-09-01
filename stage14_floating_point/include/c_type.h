#ifndef __CTYPE_H
#define __CTYPE_H

#include <stdbool.h>

#include "list_util.h"

#define RANK_CHAR 1
#define RANK_SHORT 2
#define RANK_INT 3
#define RANK_LONG 4
#define RANK_FLOAT 5
#define RANK_DOUBLE 6

typedef struct CType CType;

typedef enum {
    CTYPE_CHAR,
    CTYPE_SHORT,
    CTYPE_INT,
    CTYPE_LONG,
    CTYPE_FLOAT,
    CTYPE_DOUBLE,
    CTYPE_PTR,
    CTYPE_ARRAY,
    CTYPE_FUNCTION
} CTypeKind;

typedef struct CType CType;

DEFINE_LINKED_LIST(CType*, CType_list);

typedef struct CType {
    CTypeKind kind;
    CType * base_type;
    int size;
    int is_signed;
    int rank;
    int array_len;
    CType_list * param_types;
} CType;


extern CType CTYPE_CHAR_T;
extern CType CTYPE_SHORT_T;
extern CType CTYPE_INT_T;
extern CType CTYPE_LONG_T;
extern CType CTYPE_FLOAT_T;
extern CType CTYPE_DOUBLE_T;
extern CType CTYPE_PTR_INT_T;

CType * make_type();
CType * make_int_type(bool is_signed);
CType * make_char_type(bool is_signed);
CType * make_short_type(bool is_signed);
CType * make_long_type(bool is_signed);
CType * make_float_type(bool is_signed);
CType * make_double_type(bool is_signed);
CType * make_pointer_type(CType * base);
CType * make_array_type(CType * base, int length);
CType * make_function_type(CType * return_type, CType_list * param_types);
CType * copy_type(const CType * src);

int sizeof_ctype(CType * ctype);
//char * ctype_to_cdecl(CType * ctype);

DEFINE_LINKED_LIST(CType*, CTypePtr_list);

void free_ctype(CType * ctype);

//bool ctype_equal(const CType *a, const CType *b);
//CType *make_ptr_type(CType *base);

bool ctype_equals(CType * a, CType * b);
bool ctype_equal_or_compatible(CType * lhs, CType * rhs);
bool ctype_lists_equal(CTypePtr_list * lhs, CTypePtr_list * rhs);
bool is_castable(CType * dest, CType * source);

CType * common_type(CType *lhs, CType *rhs);
CTypePtr_list * astNodeListToTypeList(const ASTNode_list * param_list);
//char * c_type_kind_to_string(CTypeKind kind);

void ctype_to_description(CType * ctype, char * buf, size_t size);
void ctype_to_cdecl(CType * ctype, char * buf, size_t size);

bool is_integer_type(CType * ctype);
bool is_floating_point_type(CType * ctype);
bool is_array_type(CType * ctype);
bool is_function_type(CType * ctype);
bool is_pointer_type(CType * ctype);
bool is_signed_type(CType * ctype);
CType * decay_if_array(CType * ctype);
int sizeof_type(CType * ctype);
int sizeof_basetype(CType * ctype);

#endif

