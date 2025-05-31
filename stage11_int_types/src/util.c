#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "util.h"
#include "token.h"
#include "ast.h"

char * read_text_file(const char* filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char * buffer = malloc(filesize + 1);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        exit(1);
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if (read_size != filesize) {
        fprintf(stderr, "fread failed\n");
        fclose(file);
        free(buffer);
        exit(1);
    }

    buffer[filesize] = '\0';
    fclose(file);
    return buffer;

}

char* change_extension(const char* source_file, const char* new_ext) {
    const char* dot = strrchr(source_file, '.'); // find last dot
    size_t base_length = (dot) ? (size_t)(dot - source_file) : strlen(source_file);

    size_t new_length = base_length + strlen(new_ext);
    char* out_file = malloc(new_length + 1); // +1 for '\0'
    if (!out_file) {
        perror("malloc");
        exit(1);
    }

    memcpy(out_file, source_file, base_length);
    strcpy(out_file + base_length, new_ext);

    return out_file;
}

// char * my_strdup(const char* s) {
//     size_t len = strlen(s) + 1;
//     char * copy = malloc(len);
//     if (copy) {
//         memcpy(copy, s, len);
//     }
//     return copy;
// }

void error(const char* fmt, ...) {
    va_list args;

    // --- 1. Write to stderr
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    // --- 2. Echo to stdout (re-initialize args)
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    exit(1);
}

// int get_node_list_count(node_list * node_list) {

//     int arg_count = 0;
//     while(node_list) {
//         arg_count++;
//         node_list = node_list->next;
//     }
//     return arg_count;
// }

// struct node_list * create_node_list(void * node) {
//     struct node_list * list = malloc(sizeof(node_list));
//     list->node = node;
//     list->next = NULL;
//     return list;
// }

// void add_node_list(struct node_list * list, void * node) {
//     struct node_list * curr = list;
//     // move to end
//     while(curr->next) {
//         curr = curr->next;
//     }

//     // if current node has a NULL node just assign it.
//     if (!curr->node) {
//         curr->node = node;
//     }
//     else {
//         // otherwise create a new list node and link the current to it.
//         struct node_list * next = malloc(sizeof(node_list));
//         curr->next = next;
//         next->next = NULL;
//         next->node = node;
//     }

// }

// struct node_list * reverse_list(node_list * head) {
//     struct node_list * reversed = NULL;

//     for (struct node_list * current = head; current != NULL; current = current->next) {
//         struct node_list * new_node = create_node_list(current->node);
// //        new_node->node = current->node;
//         new_node->next = reversed;
//         reversed = new_node;
//     }
//     return reversed;
// }

// void free_node_list(struct node_list * list) {
//     while(list != NULL) {
//         struct node_list * next = list->next;
//         free(list);
//         list = next;
//     }
// }

