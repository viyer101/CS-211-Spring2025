#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node* next;
} Node;

//create a new node
Node* create_newnode(int value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = value;
    new_node->next = NULL;
    return new_node;
}
 
//insert a new value in sorted order without duplicates
Node* insert(Node* head, int value) {
    Node* newNode = create_newnode(value);
    if (!newNode) return head; // Memory allocation failure

    // If list is empty or new node should be the new head
    if (!head || value < head->data) {
        newNode->next = head;
       // printf("new list %d",newNode->data);
        return newNode;
    }
    
    Node* current = head;
    while (current->next && current->next->data < value) {
        current = current->next;
    }
    
    

    // If the value already exists, do not insert
    if (((current->next) && (current->next->data == value)) || (current->data==value)) {
        //printf("value exists already %d",(current->next)->data);
        free(newNode);
        return head;
    } 
    
    // Insert the new node
    newNode->next = current->next;
    current->next = newNode;
    return head;
}


//delete a value from the list
Node* delete(Node* head, int value) {
    if (head == NULL) return head; // List is empty

    // If head node needs to be deleted
    if (head->data == value) {
        Node* temp = head;
        head = head->next;
        free(temp);
        return head;
    }

    // Search for the value
    Node* current = head;
    while (current->next != NULL && current->next->data != value) {
        current = current->next;
    }

    // If found, delete the node
    if (current->next != NULL) {
        Node* temp = current->next;
        current->next = temp->next;
        free(temp);
    }

    return head;
}

// printing the list
void print_list(Node* head) {
    int count = 0;
    Node* temp = head;

    // Count nodes
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    printf("%d :", count);

    // Print nodes
    temp = head;
    while (temp != NULL) {
        printf(" %d", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

// Main function to test input files
int main() {
    Node* head = NULL;
    char command;
    int value;

    while (scanf(" %c %d", &command, &value) != EOF) {
        if (command == 'i') {
            head = insert(head, value);
        } else if (command == 'd') {
            head = delete(head, value);
        } else {
            printf("Invalid command\n");
        }
        print_list(head);
    }

    // free memory
    while (head != NULL) {
        Node* temp = head;
        head = head->next;
        free(temp);
    }

    return 0;
}