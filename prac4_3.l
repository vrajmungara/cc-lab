%{
#include <stdio.h>

int chars = 0;
int words = 0;
int lines = 0;
%}

%%
\n              { chars++; lines++; }
[ \t]           { chars++; }
[^ \t\n]+       { chars += yyleng; words++; }
%%

int main()
{
    FILE *fp = fopen("input.txt", "r");

    if (!fp) {
        printf("Error: Cannot open file.\n");
        return 1;
    }

    yyin = fp;      // Read input from file
    yylex();

    printf("Characters : %d\n", chars);
    printf("Words      : %d\n", words);
    printf("Lines      : %d\n", lines);

    fclose(fp);
    return 0;
}

int yywrap()
{
    return 1;
}
