building a c compiler based on c99 specification. 
building in stages where each implements a subset 
but includes phases to generate assembly for the
target processor (x86_64). subsequent phases build on and
add more features of the language.