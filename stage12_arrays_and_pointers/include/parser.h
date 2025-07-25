#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include "parser_context.h"

// extern char currentTokenInfo[128];
// extern char nextTokenInfo[128];

//void initialize_parser(ParserContext * parserContext, TokenList * tokenList);
//ASTNode* parse(ParserContext * parserContext);

typedef struct ParamInfo {
    char * name;
    CType * type;
    ASTNode * astNode;
} ParamInfo;

typedef struct Declarator {
    char * name;
    CType * type;
    ASTNode_list * param_list;
} Declarator;


void free_param_info(ParamInfo * param_info);

CType_list * get_ctype_list(ParamInfo_list * param_list);
ASTNode * parse(tokenlist * tokenList);
ASTNode * parse_translation_unit(ParserContext * parserContext);
ASTNode * parse_external_declaration(ParserContext * parserContext);
//ASTNode * parse_function(ParserContext * parserContext);
ASTNode * parse_function_definition(ParserContext * parserContext, const char * name, CType * ctype, ASTNode_list * param_list);
ASTNode*  parse_declaration_tail(ParserContext * parserContext, CType * ctype, const char * name);
ASTNode * parse_statement(ParserContext* parserContext);
ASTNode * parse_return_statement(ParserContext * parserContext);
ASTNode * parse_block(ParserContext* parserContext);
ASTNode * parse_if_statement(ParserContext * parserContext);
ASTNode * parse_expression_statement(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext* parserContext);
ASTNode * parse_equality_expression(ParserContext * parserContext);
ASTNode * parse_relational_expression(ParserContext * parserContext);
ASTNode * parse_additive_expression(ParserContext * parserContext);
ASTNode * parse_multiplicative_expression(ParserContext * parserContext);
ASTNode * parse_cast_expression(ParserContext * parserContext);
ASTNode * parse_unary_expression(ParserContext * parserContext);
ASTNode * parse_postfix_expression(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_constant_expression(ParserContext * parserContext);
ASTNode * parse_primary(ParserContext * parserContext);
ASTNode * parse_local_declaration(ParserContext * parserContext);
ASTNode * parse_assignment_statement(ParserContext * parserContext);
ASTNode * parse_while_statement(ParserContext * parserContext);
ASTNode * parse_for_statement(ParserContext * parserContext);
ASTNode * parse_assignment_expression(ParserContext * parserContext);
ASTNode * parse_logical_or(ParserContext * parserContext);
ASTNode * parse_logical_and(ParserContext * parserContext);
ASTNode * parse_constant_expression(ParserContext * parserContext);
ASTNode * parse_assert_extension_statement(ParserContext * parserContext);
ASTNode * parse_print_extension_statement(ParserContext * parserContext);
ASTNode * parse_constant_expression(ParserContext * parserContext);
ASTNode * parse_initializer_list(ParserContext * parserContext);

ParamInfo_list * parse_parameter_type_list(ParserContext * ctx/*, ASTNode_list ** out_params*/);

CType * parse_type_specifier(ParserContext * ctx);
Declarator * parse_declarator(ParserContext * ctx, Declarator * declarator);
Declarator * parse_direct_declarator(ParserContext * ctx, Declarator * declarator);
Declarator * parse_postfix_declarator(ParserContext * ctx, Declarator * declarator);

#endif