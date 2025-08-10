### Grammar Rules

***External Definitions***
```
<translation-unit> ::= 
      <external-declaration> 
    | <translation-unit> <external-declaration>

<external-declaration> ::= 
      <function-definition> 
    | <declaration>

<function-definition> ::= 
      <type-specifiers> <declarator> [<declaration-list>] <compound-statement>

<declaration-list> ::=
      <declaration>
    | <declaration-list> <declaration>        
```
***Declarations***
```
<declaration> ::=
      <declaration-specifiers> [<init-declaration-list>] ;

<declaration-specifiers> ::=
      <type-specifier> [<declaration-specifiers>]

<init-declaration-list> ::=
      <init-declarator>
    | <init-declarator-list> , <init-declarator>

<init-declarator> ::=
      <declarator>
    | <declarator> = <initializer>

<type-specifier> ::=
    <char>
  | <short>
  | <int>
  | <long>

<declarator> ::=
    [<pointer>] <direct-declarator>

<direct-declarator> ::=
      <identifier>
    | ( <declarator> )

<pointer> ::=
      * [<type-qualifier-list>]
    | * [<type-qualifier-list>] <pointer>

<type-qualifier-list> ::=
      <type-qualifier>
    | <type-qualifier-list> <type-qualifier>
    
  
```

***Statements***
```
<statement> ::= 
      <labeled-statement> 
    | <compound-statement> 
    | <expression-statement> 
    | <selection-statement> 
    | <iteration-statement> 
    | <jump-statement>
```
