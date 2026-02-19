%{
#include <stdio.h>
%}

%%
[0-9]+      { printf("%s\n", yytext); }   /* Match one or more digits */
.|\n        ;   /* Ignore all other characters */
%%

int main()
{
    printf("Enter a string:\n");
    yylex();        /* Start lexical analysis */
    return 0;
}

int yywrap()
{
    return 1;
}
