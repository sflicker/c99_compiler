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

#define TEST_ASSERT_EQ_INT(msg, expected, actual) \
    do { \
        printf("%s... ", msg); \
        fflush(stdout); \
        if ((expected) != (actual)) { \
            printf(COLOR_RED "FAILED\n" COLOR_RESET); \
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
            fprintf(stderr, "    Expected: \"%s\"\n    Actual:   \"%s\"\n", (expected), (actual)); \
            exit(1); \
        } else { \
            printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(msg, ptr) \
    do { \
        printf("%s... ", msg); \
        fflush(stdout); \
        if ((ptr) == NULL) { \
            printf(COLOR_RED "FAILED\n" COLOR_RESET); \
            fprintf(stderr, "    Pointer is NULL\n"); \
            exit(1); \
        } else { \
            printf(COLOR_GREEN "Passed\n" COLOR_RESET); \
        } \
    } while (0)

#endif