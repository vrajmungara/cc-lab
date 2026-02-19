%{
#include<stdio.h>
#include<string.h>

int line = 1;

/* Symbol Table */
char symbol_table[100][50];
int sym_count = 0;

/* Check duplicate identifier */
int check_symbol(char str[])
{
    int i;
    for(i = 0; i < sym_count; i++)
    {
        if(strcmp(symbol_table[i], str) == 0)
            return 1;
    }
    return 0;
}
%}

/* -------- Definitions -------- */

KEYWORD int|char|float|double|if|else|for|while|return|void|long|short|struct
ID [a-zA-Z_][a-zA-Z0-9_]*
NUM [0-9]+
CHAR \'[^\']\'
STRING \"[^\"]*\"
OP [+\-*/=<>]
PUNC [;,(){}]

%%

"//".*        ;

"/*"          {
                char c;
                while((c = input()) != 0)
                {
                    if(c == '*')
                    {
                        if(input() == '/')
                            break;
                    }
                }
              }

[0-9]+[a-zA-Z]+   {
                    printf("Line %d : %s invalid lexeme\n", line, yytext);
                  }

{KEYWORD}   { printf("Keyword: %s\n", yytext); }

{CHAR}      { printf("Constant: %s\n", yytext); }

{STRING}    { printf("String: %s\n", yytext); }

{NUM}       { printf("Constant: %s\n", yytext); }

{ID}        {
                printf("Identifier: %s\n", yytext);

                if(!check_symbol(yytext))
                {
                    strcpy(symbol_table[sym_count], yytext);
                    sym_count++;
                }
            }

{OP}        { printf("Operator: %s\n", yytext); }

{PUNC}      { printf("Punctuation: %s\n", yytext); }

\n          { line++; }

[ \t]+      ;

.           { printf("Line %d : %s invalid lexeme\n", line, yytext); }

%%

int main()
{
    printf("TOKENS\n");
    yylex();

    printf("\nSYMBOL TABLE ENTRIES\n");
    int i;
    for(i = 0; i < sym_count; i++)
    {
        printf("%d) %s\n", i+1, symbol_table[i]);
    }

    return 0;
}

int yywrap()
{
    return 1;
}