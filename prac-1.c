#include <stdio.h>
#include <string.h>

int main() {
    char str[100];
    int i = 0, len;

    printf("Enter string: ");
    scanf("%s", str);

    len = strlen(str);

    // Length must be at least 2 for "bb"
    if (len < 2) {
        printf("Invalid String");
        return 0;
    }

    // Check ending "bb"
    if (str[len - 1] != 'b' || str[len - 2] != 'b') {
        printf("Invalid String");
        return 0;
    }

    // Check remaining characters are only 'a'
    for (i = 0; i < len - 2; i++) {
        if (str[i] != 'a') {
            printf("Invalid String");
            return 0;
        }
    }

    printf("Valid String");
    return 0;
}

