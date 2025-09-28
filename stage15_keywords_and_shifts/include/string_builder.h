//
// Created by scott on 8/12/25.
//

#ifndef _STRING_BUILDER_H
#define _STRING_BUILDER_H
#include <stddef.h>


typedef struct String_Builder{
    char * buffer;
    size_t length;
    size_t capacity;
} String_Builder;

String_Builder * create_string_builder();
String_Builder * append_string(String_Builder * string_builder,char * string);
String_Builder * append_char(String_Builder * string_builder,char c);
void free_string_builder(String_Builder * string_builder);
char * build_string(String_Builder * string_builder);

#endif //_STRING_BUILDER_H