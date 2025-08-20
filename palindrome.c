#include <stdio.h>
#include <ctype.h>
#include <string.h>

// Function to check if a string is a palindrome.
int is_palindrome(const char *str) {
    int left = 0, right = strlen(str) - 1;
    while (left < right) 
    {
        // Move left index to next letter
        while (left < right && !isalpha(str[left])) 
            left++;
        

        // Move right index to previous letter
        while (left < right && !isalpha(str[right])) 
            right--;
        
        // Compare characters (isn't case sensitive)
        if (tolower(str[left]) != tolower(str[right])) 
            return 0; // Not a palindrome
        
        left++;
        right--;
    }

    return 1; // returns 1 if it a palindrome
}

int main(int argc, char *argv[]) {
    if (argc != 2) //checks whether arguments passed is not equal to 2
    {
        printf("Usage: %s <string>\n", argv[0]);
        return 1;
    }

    if (is_palindrome(argv[1])) //checks whether the string is a palindrome
        printf("yes\n");
    else 
        printf("no\n");
    
    return 0;
}

