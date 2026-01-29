#include <bits/stdc++.h>
using namespace std;

string input;
int i = 0;

void error() {
    cout << "Syntax Error" << endl;
    exit(0);
}


void match(char expected) {
    if (input[i] == expected)
        i++;
    else
        error();
}

/
void E_dash() {
    if (input[i] == '+') {
        match('+');

       
        if (isalpha(input[i]))
            match(input[i]);
        else
            error();

        E_dash();
    }
   
}


void E() {
    if (isalpha(input[i])) {
        match(input[i]); 
        E_dash();
    } else {
        error();
    }
}

int main() {
    cout << "Enter the input string: ";
    cin >> input;
    input += '$'; 

    E();

    if (input[i] == '$')
        cout << "Valid String" << endl;
    else
        cout << "Invalid String" << endl;

    return 0;
}
