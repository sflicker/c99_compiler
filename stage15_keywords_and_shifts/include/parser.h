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




void free_param_info(ParamInfo * param_info);

CType_list * get_ctype_list(ParamInfo_list * param_list);
ASTNode * parse(tokenlist * tokenList);
ASTNode * parse_translation_unit(ParserContext * parserContext);
//ASTNode * parse_function(ParserContext * parserContext);
ASTNode * parse_function_definition(ParserContext * parserContext, const char * name, CType * ctype, ASTNode_list * param_list);
ASTNode * parse_statement(ParserContext* parserContext);
ASTNode * parse_return_statement(ParserContext * parserContext);
ASTNode * parse_block(ParserContext* parserContext);
ASTNode * parse_if_statement(ParserContext * parserContext);
ASTNode * parse_expression_statement(ParserContext * parserContext);
ASTNode * parse_assignment_statement(ParserContext * parserContext);
ASTNode * parse_while_statement(ParserContext * parserContext);
ASTNode * parse_for_statement(ParserContext * parserContext);
ASTNode * parse_assert_extension_statement(ParserContext * parserContext);
ASTNode * parse_print_extension_statement(ParserContext * parserContext);

ParamInfo_list * parse_parameter_type_list(ParserContext * ctx/*, ASTNode_list ** out_params*/);


#endif