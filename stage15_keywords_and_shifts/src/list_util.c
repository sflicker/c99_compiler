//
// Created by scott on 6/19/25.
//
#include "list_util.h"

void reverse_ASTNode_list(ASTNode_list * list) {
    ASTNode_list_node * prev = NULL;
    ASTNode_list_node * head = NULL;
    ASTNode_list_node * original_head = list->head;
    ASTNode_list_node * next = NULL;

    head = list->head;
    while (head != NULL) {
        next = head->next;
        prev = head;
        head = next;
    }

    list->head = prev;
    list->tail = original_head;
}