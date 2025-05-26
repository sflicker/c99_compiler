building a c compiler based on c99 specification. 
building in stages where each implements a subset 
but includes phases to generate assembly for the
target processor (x86_64). subsequent phases build on and
add more features of the language.

Stage 1 at this phase the compiler will handle a 
single function only int data type and the only supported 
statement will be a return of int literals. This
will allow testing the executable by asserting 
the correct return value.

Stage 2 added expression support for basic arithmetic.
      +,_,*,\ along with (). The program will 
return the result after evaluating the expression.

