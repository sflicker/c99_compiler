#pragma once

#include <stdbool.h>

void error(const char* fmt, ...);
void warning(const char* fmt, ...);

void set_error_exit_on_error_enabled(bool enabled);
bool error_exit_enabled();
bool error_occurred();
const char* error_message();
void clear_error_state();
