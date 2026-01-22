#include <stdio.h>

int main() {
    int states, symbols;
    int start, acceptCount;
    int transition[10][10];
    int accept[10];
    char input[100];
    int current, i, j;

    printf("Enter number of states: ");
    scanf("%d", &states);

    printf("Enter number of input symbols: ");
    scanf("%d", &symbols);

    printf("Enter transition table:\n");
    for (i = 0; i < states; i++) {
        for (j = 0; j < symbols; j++) {
            scanf("%d", &transition[i][j]);
        }
    }

    printf("Enter start state: ");
    scanf("%d", &start);

    printf("Enter number of accepting states: ");
    scanf("%d", &acceptCount);

    printf("Enter accepting states: ");
    for (i = 0; i < acceptCount; i++) {
        scanf("%d", &accept[i]);
    }

    printf("Enter input string (symbols as numbers 0/1): ");
    scanf("%s", input);

    current = start;

    for (i = 0; input[i] != '\0'; i++) {
        int sym = input[i] - '0';
        current = transition[current][sym];
    }

    for (i = 0; i < acceptCount; i++) {
        if (current == accept[i]) {
            printf("Valid String");
            return 0;
        }
    }

    printf("Invalid String");
    return 0;
}
