#ifndef __UTIL_H__
#define __UTIL_H__

typedef struct node_list {
    void * node;
    struct node_list * next;
} node_list;

int get_node_list_count(struct node_list * node_list);
struct node_list * create_node_list(void * node);
void add_node_list(struct node_list * list, void * node);
struct node_list * reverse_list(node_list * head);
void free_node_list(struct node_list * list);

char * my_strdup(const char* s);
void error(const char * message);

#endif