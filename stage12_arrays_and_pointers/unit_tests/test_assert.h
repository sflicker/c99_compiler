#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"

#define TEST_USE_COLOR 1

#if TEST_USE_COLOR
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m]"
#define COLOR_CYAN    "\033[36m"
#else
#define COLOR_RESET   ""
#define COLOR_GREEN   ""
#define COLOR_RED     ""
#define COLOR_CYAN    ""
#endif



#define TEST_MSG(msg)   \
    do {                \
        printf(COLOR_CYAN "[%s] " COLOR_RESET "%s...\n", current_test, msg);    \
    } while (0)

#define TEST_ASSERT(msg, expr)  \
    do {                         \
         printf(COLOR_CYAN "[%s] " COLOR_RESET "%s...", current_test, msg); \
         if (expr) {                                                  \
                printf(COLOR_GREEN "Passed" COLOR_RESET "\n");        \
         } else {                                                     \
                printf(COLOR_RED "FAILED" COLOR_RESET "\n");          \
                exit(1);                                              \
        }                                                             \
    } while (0)

#define TEST_ASSERT_EQ_INT(msg, expected, actual) \
    do { \
        printf("%s... ", msg); \
        fflush(stdout); \
        if ((expected) != (actual)) { \
            printf(COLOR_RED "FAILED\n" COLOR_RESET); \
            printf("  Expected:", expected); \
            printf("  Actual: ", actual); \
            fprintf(stderr, "    Expected: %d\n    Actual:   %d\n", (expected), (actual)); \
            exit(1); \
        } else { \
            printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
        } \
    } while (0)

#define TEST_ASSERT_EQ_STR(msg, expected, actual) \
    do { \
        printf("%s... ", msg); \
        fflush(stdout); \
        if (strcmp((expected), (actual)) != 0) { \
            printf(COLOR_RED "FAILED\n" COLOR_RESET); \
            print_with_label("Expected", expected); \
            print_with_label("Actual", actual ); \
            exit(1); \
        } else { \
            printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(msg, ptr) \
    do { \
        printf("%s... ", msg "\n"); \
        fflush(stdout); \
        if ((ptr) == NULL) { \
            printf(COLOR_RED "FAILED\n" COLOR_RESET); \
            fprintf(stderr, "    Pointer is NULL\n"); \
            exit(1); \
        } else { \
            printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
        } \
    } while (0)

#define RUN_TEST(fn)                     \
    do {                                 \
        current_test = #fn;              \
        fn();                            \
    } while (0)

#endif