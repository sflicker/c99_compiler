//
// Created by scott on 7/20/25.
//
#include "test_assert.h"

#include "ast.h"
#include "parser_util.h"

const char * current_test = NULL;

void test_flatten_list() {
    ASTNode_list * outer_list = create_node_list() ;
    ASTNode_list * inner_list = create_node_list() ;
    ASTNode_list_append(inner_list, create_int_literal_node(10));
    ASTNode_list_append(inner_list, create_int_literal_node(20));

    ASTNode_list_append(outer_list, create_initializer_list(inner_list));

    inner_list = create_node_list() ;

    ASTNode_list_append(inner_list, create_int_literal_node(30));
    ASTNode_list_append(inner_list, create_int_literal_node(40));

    ASTNode_list_append(outer_list, create_initializer_list(inner_list));

    ASTNode_list * flattened_list = create_node_list() ;
    flatten_list(outer_list, flattened_list);

    TEST_ASSERT("Verifying flattened list has correct number of items", flattened_list->count == 4);

}

int main() {
    RUN_TEST(test_flatten_list);
}