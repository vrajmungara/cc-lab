%{
#include <stdio.h>
#include <string.h>
%}

%%
.*\n   {
        char pwd[100];
        strcpy(pwd, yytext);

        int len = strlen(pwd);

        /* remove newline from length */
        if(pwd[len-1] == '\n') {
            pwd[len-1] = '\0';
            len--;
        }

        int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSymbol = 0;

        if(len >= 9 && len <= 15) {

            for(int i = 0; i < len; i++) {
                if(pwd[i] >= 'a' && pwd[i] <= 'z') hasLower = 1;
                else if(pwd[i] >= 'A' && pwd[i] <= 'Z') hasUpper = 1;
                else if(pwd[i] >= '0' && pwd[i] <= '9') hasDigit = 1;
                else if(pwd[i]=='*' || pwd[i]==';' || pwd[i]=='#' || pwd[i]=='$' || pwd[i]=='@')
                    hasSymbol = 1;
            }

            if(hasLower && hasUpper && hasDigit && hasSymbol)
                printf("Valid password\n");
            else
                printf("Invalid password\n");
        }
        else {
            printf("Invalid password\n");
        }
      }
%%

int main()
{
    printf("Enter password:\n");
    yylex();
    return 0;
}

int yywrap()
{
    return 1;
}
