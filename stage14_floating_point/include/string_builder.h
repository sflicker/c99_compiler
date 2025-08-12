//
// Created by scott on 8/12/25.
//

#ifndef _STRING_BUILDER_H
#define _STRING_BUILDER_H
#include "list_util.h"

DEFINE_LINKED_LIST(char, char_list);

typedef struct String_Builder{
    char_list* string;

} String_Builder;
String_Builder * create_string_builder();
String_Builder * string_builder_init(String_Builder * string_builder,char_list string);
String_Builder * append_string(String_Builder * string_builder,char * string);
String_Builder * append_char(String_Builder * string_builder,char c);
void destroy_string_builder(String_Builder * string_builder);
char * string_builder_to_string(String_Builder * string_builder);

#endif //_STRING_BUILDER_H