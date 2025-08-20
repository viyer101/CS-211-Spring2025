#include <stdio.h>
#include <ctype.h>

void rot13(const char *input) { //helper void function that uses standard output to print out the alphabetic character shifted 13 spaces
    char character; //declares a character 
    while ((character = *input++)) //checks if character is inside the unchanged char input.
    {  
        if (isalpha(character)) //checks if the character is an alphabetical character
        { 
            char baseChar = (character >= 'a' && character <= 'z') ? 'a' : 'A'; //sets the base alphabetic character between char a and z
            putchar(baseChar + (character - baseChar + 13) % 26); //prints out the alphabetical string shifted by 13 spaces.
        } 
        else 
            putchar(character); //otherwise if the character is not alphabetical it just prints the character through standard input.
    }

    putchar('\n'); //prints newline character
}

int main(int argc, char *argv[]) {

    if (argc != 2) 
        return 1; // Expects exactly one argument
    
    rot13(argv[1]); //calls the rot13 function passing an char argument.
    return 0;
}