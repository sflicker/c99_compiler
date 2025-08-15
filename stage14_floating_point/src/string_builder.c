

#include <stdlib.h>
#include <string.h>

#include "string_builder.h"

int STRING_BUILDER_DEFAULT_CAPACITY = 128;

String_Builder * create_string_builder() {
    String_Builder * builder = (String_Builder *)malloc(sizeof(String_Builder));
    builder->length = 0;
    builder->capacity = STRING_BUILDER_DEFAULT_CAPACITY;
    builder->buffer = (char *)malloc(builder->capacity);
    memset(builder->buffer, 0, builder->capacity);
    return builder;
}

String_Builder * grow_string_builder(String_Builder * builder) {
    char * current = builder->buffer;
    builder->capacity *= 2;
    builder->buffer = (char *)malloc(builder->capacity);
    memset(builder->buffer, 0, builder->capacity);
    memcpy(builder->buffer, current, builder->length);
    free(current);
    return builder;
}

void free_string_builder(String_Builder * builder) {
    free(builder->buffer);
}

String_Builder * append_string(String_Builder * string_builder,char * string) {
    int new_length = string_builder->length + strlen(string);
    if (new_length > string_builder->capacity) {
        string_builder = grow_string_builder(string_builder);
    }
    memcpy(string_builder->buffer + string_builder->length, string, strlen(string));
    string_builder->length = new_length;
    return string_builder;
}

String_Builder * append_char(String_Builder * string_builder,char c) {
    int new_length = string_builder->length + 1;
    if (new_length > string_builder->capacity) {
        string_builder = grow_string_builder(string_builder);
    }
    memcpy(string_builder->buffer + string_builder->length, &c, 1);
    string_builder->length = new_length;
    return string_builder;
}

char * build_string(String_Builder * string_builder) {
    return strdup(string_builder->buffer);
}
