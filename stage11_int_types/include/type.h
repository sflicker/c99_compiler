#ifndef __TYPE_H
#define __TYPE_H

typedef enum {
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG
} TypeKind;

typedef struct {
    TypeKind kind;
    int size;
    int is_signed;
} Type;

extern Type TYPE_CHAR_T;
extern Type TYPE_SHORT_T;
extern Type TYPE_INT_T;
extern Type TYPE_LONG_T;

int sizeof_type(Type * type);
const char * type_name(Type * type);


#endif

