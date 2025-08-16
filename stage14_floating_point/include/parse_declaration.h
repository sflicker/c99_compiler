//
// Created by scott on 8/16/25.
//

#ifndef _PARSE_DECLARATION_H
#define _PARSE_DECLARATION_H

typedef struct Declarator {
    char * name;
    CType * type;
    ASTNode_list * param_list;
} Declarator;

ASTNode * parse_external_declaration(ParserContext * parserContext);
Declarator * parse_declarator(ParserContext * ctx, Declarator * declarator);
CType * parse_type_specifier(ParserContext * ctx);
Declarator * parse_direct_declarator(ParserContext * ctx, Declarator * declarator);
Declarator * parse_postfix_declarator(ParserContext * ctx, Declarator * declarator);
CType * parse_abstract_declarator(ParserContext * ctx, CType * base);
ASTNode*  parse_declaration_tail(ParserContext * parserContext, CType * ctype, const char * name);
ASTNode * parse_local_declaration(ParserContext * parserContext);
ASTNode * parse_initializer_list(ParserContext * parserContext);


#endif //MIMIC99_STAGE14_PARSE_DECLARATION_H