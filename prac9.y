
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *msg);

char inputbuf[256];
int  bufpos;
%}

%token TOK_I TOK_E TOK_T TOK_A TOK_B

%nonassoc LOWER_THAN_ELSE
%nonassoc TOK_E

%%

program
    : S   { printf("Valid string\n\n"); }
    ;

S
    : TOK_I E TOK_T S %prec LOWER_THAN_ELSE
    | TOK_I E TOK_T S TOK_E S
    | TOK_A
    ;

E
    : TOK_B
    ;

%%

int yylex(void) {
    while (inputbuf[bufpos] == ' ' || inputbuf[bufpos] == '\t')
        bufpos++;

    char c = inputbuf[bufpos];

    if (c == '\0' || c == '\n')
        return 0;

    bufpos++;

    switch (c) {
        case 'i': return TOK_I;
        case 'e': return TOK_E;
        case 't': return TOK_T;
        case 'a': return TOK_A;
        case 'b': return TOK_B;
        default:
            return -1;
    }
}

void yyerror(const char *msg) {
    printf("Invalid string\n\n");
}

int main(void) {
    printf("============================================\n");
    printf("  String Parsing Using YACC\n");
    printf("  Grammar: S->iEtSS'|a  S'->eS|e  E->b\n");
    printf("  Type 'exit' to quit.\n");
    printf("============================================\n\n");

    while (1) {
        printf("Enter string: ");
        fflush(stdout);

        if (fgets(inputbuf, sizeof(inputbuf), stdin) == NULL)
            break;

        /* Remove trailing newline */
        inputbuf[strcspn(inputbuf, "\n")] = '\0';

        if (strcmp(inputbuf, "exit") == 0)
            break;

        bufpos = 0;
        yyparse();
    }

    printf("Goodbye!\n");
    return 0;
}
