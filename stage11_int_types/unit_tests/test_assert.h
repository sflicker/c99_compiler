#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST_USE_COLOR 1

#if TEST_USE_COLOR
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m]"
#else
#define COLOR_RESET   ""
#define COLOR_GREEN   ""
#define COLOR_RED     ""
#endif

#define TEST_ASSERT(msg, expr) \
    do { \
         printf("%s... ", msg); \
         fflush(stdout); \
         assert(expr); \
         printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
    } while (0)



#endif