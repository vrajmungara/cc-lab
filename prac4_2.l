%{
#include <stdio.h>
%}

%%
[cC][hH][aA][rR][uU][sS][aA][tT]    { printf("university"); }
\n                                  { printf("\n"); }
.                                   { printf("%s", yytext); }
%%

int main()
{
    printf("Enter text:\n");
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}
