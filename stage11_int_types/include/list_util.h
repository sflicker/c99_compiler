#ifndef LIST_UTIL_H
#define LIST_UTIL_H

#include <stdio.h>
#include <stdlib.h>

// Helper to expand free_fn only if it's not NULL
#define FREE_IF_DEFINED(fn, val)                                 \
    do { if ((fn) != NULL) fn(val); } while (0)

// type = base type (e.g., ASTNode*), name = prefix (e.g., arglist)
#define DEFINE_LINKED_LIST(type, name)                                       \
                                                                              \
typedef struct name##_node {                                                 \
    type value;                                                              \
    struct name##_node* next;                                                \
} name##_node;                                                               \
                                                                              \
typedef struct {                                                              \
    name##_node* head;                                                       \
    name##_node* tail;                                                       \
    int count;                                                               \
    void (*free_fn)(type);                                                   \
} name;                                                                      \
                                                                             \
typedef struct {                                                             \
    name##_node* current;                                                    \
} name##_cursor;                                                             \
                                                                              \
static inline void name##_init(name* list, void(*fn)(type)) {                \
    list->head = list->tail = NULL;                                          \
    list->count = 0;                                                         \
    list->free_fn = fn;                                                      \
}                                                                             \
                                                                              \
static inline void name##_append(name* list, type value) {                    \
    name##_node* node = malloc(sizeof(name##_node));                         \
    node->value = value;                                                     \
    node->next = NULL;                                                       \
    if (list->tail) {                                                        \
        list->tail->next = node;                                             \
    } else {                                                                 \
        list->head = node;                                                   \
    }                                                                        \
    list->tail = node;                                                       \
    list->count++;                                                           \
}                                                                             \
                                                                             \
static inline void name##_cursor_init(name##_cursor* cursor, name *list) {   \
    cursor->current = list->head;                                            \
}                                                                            \
                                                                             \
static inline int name##_cursor_has_next(name##_cursor * cursor) {           \
    return cursor->current != NULL;                                          \
}                                                                            \
                                                                             \
static inline type name##_cursor_next(name##_cursor* cursor) {               \
    if (cursor->current) {                                                   \
        type val = cursor->current->value;                                   \
        cursor->current = cursor->current->next;                             \
        return val;                                                          \
    }                                                                        \
    return NULL;                                                             \
}                                                                            \
                                                                             \
static inline type name##_cursor_peek_next(name##_cursor * cursor) {         \
    if (cursor->current) {                                                   \
        name##_node * current = cursor->current;                             \
        if (current->next) {                                                 \
            name##_node * next = current->next;                              \
            return next->value;                                              \
        }                                                                    \
    }                                                                        \
    return NULL;                                                             \
}                                                                            \
                                                                             \
static inline void name##_free(name* list) {                                  \
    if (!list) return;                                                       \
    name##_node * curr = list->head;                                         \
    while(curr) {                                                            \
        name##_node* next = curr->next;                                      \
        if (list->free_fn) list->free_fn(curr->value);                       \
        free(curr);                                                          \
        curr = next;                                                         \
    }                                                                        \
    list->head = list->tail = NULL;                                          \
    list->count = 0;                                                         \
                                                                             \
}

typedef struct ASTNode ASTNode;

DEFINE_LINKED_LIST(ASTNode*, ASTNode_list);



#endif // LIST_UTIL_H
