#ifndef LIST_UTIL_H
#define LIST_UTIL_H

#include <stdlib.h>

// type = base type (e.g., ASTNode*), name = prefix (e.g., arglist)

#define DEFINE_LINKED_LIST(type, name)                                        \
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
} name;                                                                      \
                                                                              \
static inline void name##_init(name* list) {                                  \
    list->head = list->tail = NULL;                                          \
    list->count = 0;                                                         \
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
static inline void name##_free(name* list, void (*free_fn)(type)) {          \
    name##_node* curr = list->head;                                          \
    while (curr) {                                                           \
        name##_node* next = curr->next;                                      \
        if (free_fn) free_fn(curr->value);                                   \
        free(curr);                                                          \
        curr = next;                                                         \
    }                                                                        \
    list->head = list->tail = NULL;                                          \
    list->count = 0;                                                         \
}                                                                             
                                                                              
/* foreach macro usage: name##_foreach(list, it) { ... use it ... } */       
#define name##_foreach(listptr, it)                                           \
    for (name##_node* it##_node = (listptr)->head;                           \
         it##_node != NULL && ((it = it##_node->value), 1);                  \
         it##_node = it##_node->next)                                        \

#endif // LIST_UTIL_H
