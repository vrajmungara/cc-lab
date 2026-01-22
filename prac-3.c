#include <stdio.h>
#include <ctype.h>
#include <string.h>

char keywords[10][10] = {
    "int", "char", "return", "void", "float",
    "if", "else", "for", "while", "struct"
};

int isKeyword(char *word) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int main() {
    FILE *fp;
    char ch, buffer[50];
    int i = 0, line = 1;

    fp = fopen("input.c", "r");

    if (fp == NULL) {
        printf("File not found");
        return 0;
    }

    printf("TOKENS\n");

    while ((ch = fgetc(fp)) != EOF) {

        if (ch == '\n') line++;

        // Identifier or Keyword
        if (isalpha(ch)) {
            buffer[i++] = ch;
            while (isalnum(ch = fgetc(fp)))
                buffer[i++] = ch;

            buffer[i] = '\0';
            i = 0;
            fseek(fp, -1, SEEK_CUR);

            if (isKeyword(buffer))
                printf("Keyword: %s\n", buffer);
            else
                printf("Identifier: %s\n", buffer);
        }

        // Numbers
        else if (isdigit(ch)) {
            buffer[i++] = ch;
            while (isalnum(ch = fgetc(fp)))
                buffer[i++] = ch;

            buffer[i] = '\0';
            i = 0;

            if (isalpha(ch))
                printf("Line %d : %s invalid lexeme\n", line, buffer);
            else {
                printf("Constant: %s\n", buffer);
                fseek(fp, -1, SEEK_CUR);
            }
        }

        // Operators
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=') {
            printf("Operator: %c\n", ch);
        }

        // Punctuation
        else if (ch == ';' || ch == ',' || ch == '(' || ch == ')' ||
                 ch == '{' || ch == '}') {
            printf("Punctuation: %c\n", ch);
        }
    }

    fclose(fp);
    return 0;
}
