#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"

char * my_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char * copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

void error(const char * message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int get_node_list_count(struct node_list * node_list) {
    int arg_count = 0;
    while(node_list) {
        arg_count++;
        node_list = node_list->next;
    }
    return arg_count;
}

struct node_list * create_node_list() {
    struct node_list * list = malloc(sizeof(node_list));
    list->node = NULL;
    list->next = NULL;
    return list;
}

void add_node_list(struct node_list * list, void * node) {
    struct node_list * curr = list;
    // move to end
    while(curr->next) {
        curr = curr->next;
    }

    // if current node has a NULL node just assign it.
    if (!curr->node) {
        curr->node = node;
    }
    else {
        // otherwise create a new list node and link the current to it.
        struct node_list * next = create_node_list();
        curr->next = next;
        next->node = NULL;
        next->node = node;
    }

}
