//
// Created by scott on 6/14/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"

static bool exit_on_error = true;   // call set_error_exit_enabled(false) to disable for testing
static bool error_flag = false;
static char error_message_buf[1024];

void set_error_exit_on_error_enabled(bool flag) {
    exit_on_error = flag;
}

bool error_occurred() {return error_flag;}

const char* error_message() {
    return error_flag ? error_message_buf : NULL;
}

void clear_error_state() {
    error_flag = false;
    error_message_buf[0] = '\0';
}

void error(const char* fmt, ...) {
    va_list args;

    error_flag = true;

    // --- 1. Write error message to buffer
    va_start(args, fmt);
    vsnprintf(error_message_buf, 1024, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", error_message_buf);
    fprintf(stdout, "%s\n", error_message_buf);

    if (exit_on_error) {
        exit(1);
    }
}
