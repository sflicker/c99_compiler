//
// Created by scott on 7/31/25.
//
#include <stdio.h>
#include "test_assert.h"

#include "ast.h"
#include "parser_util.h"
#include "c_type.h"
#include "emitter.h"
#include "emitter_context.h"
#include "emitter_helpers.h"

const char * current_test = NULL;

void test_escaped_string() {
    char * input = "hello world\n";
    char * expected = "\"hello world\\n\"";
    char * output = escaped_string(input);

    TEST_ASSERT("Verifying string is properly escaped", strcmp(output, expected) == 0);
}

void test_emit_load_from__char() {
    CType * ctype = make_char_type(false);
    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * mem_file = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(mem_file);

    emit_load_from(emitter_context, ctype, "rcx");

    emitter_finalize(emitter_context);
    char * normalizedOutput = malloc(strlen(buffer) + 1);
    strip_comments(buffer, normalizedOutput);

    char * expected = "movsx eax, byte [rcx]\n";

    TEST_ASSERT("Verifying correct load_from for char", strcmp(normalizedOutput, expected) == 0);
}

void test_emit_load_from__short() {
    CType * ctype = make_short_type(false);
    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * mem_file = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(mem_file);

    emit_load_from(emitter_context, ctype, "rcx");

    emitter_finalize(emitter_context);
    char * normalizedOutput = malloc(strlen(buffer) + 1);
    strip_comments(buffer, normalizedOutput);

    char * expected = "movsx eax, word [rcx]\n";

    TEST_ASSERT("Verifying correct load_from for short", strcmp(normalizedOutput, expected) == 0);
}

void test_emit_load_from__int() {
    CType * ctype = make_int_type(false);
    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * mem_file = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(mem_file);

    emit_load_from(emitter_context, ctype, "rcx");

    emitter_finalize(emitter_context);
    char * normalizedOutput = malloc(strlen(buffer) + 1);
    strip_comments(buffer, normalizedOutput);

    char * expected = "mov eax, [rcx]\n";

    TEST_ASSERT("Verifying correct load_from for int", strcmp(normalizedOutput, expected) == 0);
}

void test_emit_load_from__long() {
    CType * ctype = make_long_type(false);
    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * mem_file = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(mem_file);

    emit_load_from(emitter_context, ctype, "rcx");

    emitter_finalize(emitter_context);
    char * normalizedOutput = malloc(strlen(buffer) + 1);
    strip_comments(buffer, normalizedOutput);

    char * expected = "mov rax, [rcx]\n";

    TEST_ASSERT("Verifying correct load_from for long", strcmp(normalizedOutput, expected) == 0);
}

int main() {
    RUN_TEST(test_escaped_string);
    RUN_TEST(test_emit_load_from__char);
    RUN_TEST(test_emit_load_from__short);
    RUN_TEST(test_emit_load_from__int);
    RUN_TEST(test_emit_load_from__long);
}