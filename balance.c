#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 1000

// Stack structure
typedef struct {
    char data[MAX_STACK_SIZE];
    int top;
} Stack;

// Stack functions

void pushFunction(Stack *s, char c) {
    if (s->top < MAX_STACK_SIZE - 1) { //checks if the stack is not full
        s->data[++s->top] = c; //pushes the character c onto the stack 
    }
}

char popFunction(Stack *s) {
    if (s->top >= 0) { //checks if the stack is not empty
        return s->data[s->top--]; //pops the top character from the stack 
    }
    return '\0'; // Empty stack

}

char peekFunction(Stack *s) { //returns the top character of the stack 
    if (s->top >= 0) { //checks if the stack is not empty
        return s->data[s->top]; // returns the top character of the stack 
    }
    return '\0';
}

int is_matching_pair(char open, char close) { //checks if the open and close characters are matching pairs

    return (open == '(' && close == ')') || //returns 1 if the open and close characters are matching pairs
           (open == '[' && close == ']') ||
           (open == '{' && close == '}');
}

int main(int argc, char *argv[]) {
    if (argc != 2) { //checks if the number of arguments passed is not equal to 2
        fprintf(stderr, "Usage: %s '<string>'\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *input = argv[1]; //sets the input character pointer to the second argument passed
    Stack stack = {.top = -1}; //creates, declares and initializes a stack 
    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i]; //creates declares and intializes the character c
        if (c == '(' || c == '[' || c == '{') {
            pushFunction(&stack, c); //calls the push function to push character c onto stack

        } 
        else if (c == ')' || c == ']' || c == '}') {
            char top = popFunction(&stack); //calls the pop function to remove the first character from the stack
            if (top == '\0' || !is_matching_pair(top, c)) {
                printf("%d: %c\n", i, c);
                return EXIT_FAILURE;
            }
        }
    }

    if (stack.top >= 0) {
        printf("open: ");
        while (stack.top >= 0) { //while the stack is NOT empty
            char open = popFunction(&stack); //calls the pop function to remove the first character, open, from the stack
            if (open == '(') printf(")");
            else if (open == '[') printf("]");
            else if (open == '{') printf("}");
        }
        printf("\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}