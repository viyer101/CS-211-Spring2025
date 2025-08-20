#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *left, *right;
} Node;

// Function to create a new node for BST
Node* create_node(int value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->value = value;
    new_node->left = new_node->right = NULL;
    return new_node;
}

// Insert function for BST
Node* insert(Node* root, int value, int* inserted) {
    if (root == NULL) {
        *inserted = 1;
        return create_node(value);
    }

    if (value < root->value) //if the value of the child is less than the value of the parent
        root->left = insert(root->left, value, inserted); //inserts it to the left of the tree
    else if (value > root->value) //if the value of the child is greater than the value of the parent
        root->right = insert(root->right, value, inserted); //inserts it to the right of the tree
    return root;
}

// Search function for BST
int search(Node* root, int value) {
    if (root == NULL) return 0; //if root does not exist
    if (root->value == value) return 1; //if the value of the root is equal to the search value 
    if (value < root->value) return search(root->left, value); //if search value is less than the value of root then traverse left
    return search(root->right, value); //recursively calls search if the search value is greater than the value of root
}

// Find the maximum value in a subtree of a BST
Node* find_max(Node* root) {
    while (root->right != NULL) root = root->right;
    return root;
}

// Delete function for BST
Node* delete(Node* root, int value, int* deleted) {
    if (root == NULL) return NULL;
    if (value < root->value) //checks if search value is less than the value of the root
        root->left = delete(root->left, value, deleted); //traverses to left of tree and deletes it
    else if (value > root->value) //checks if search value is greater than the value of the root
        root->right = delete(root->right, value, deleted); //traveses to right of tree and deletes it
    else {
        *deleted = 1;
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } 
        else if (root->right == NULL) 
        {
            Node* temp = root->left;
            free(root);
            return temp;
        }

        Node* max_left = find_max(root->left);
        root->value = max_left->value;
        root->left = delete(root->left, max_left->value, deleted); //recursively calls delete to delete the maximum left node
    }

    return root;
}

// Print function that prints the binary search tree
void print_tree(Node* root) {
    if (root == NULL) return;
    printf("(");
    print_tree(root->left);
    printf("%d", root->value);
    print_tree(root->right);
    printf(")");
}

// Free memory
void free_tree(Node* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main() {
    Node* root = NULL;
    char command;
    int value;

    while (scanf(" %c", &command) == 1) {
        if (command == 'i') {
            if (scanf("%d", &value) == 1) {
                int inserted = 0;
                root = insert(root, value, &inserted);
                printf("%s\n", inserted ? "inserted" : "not inserted");
            }
        } 

        else if (command == 's') 
        {
            if (scanf("%d", &value) == 1) 
                printf("%s\n", search(root, value) ? "present" : "absent");
        } 

        else if (command == 'p') 
        {
            print_tree(root);
            printf("\n");
        } 

        else if (command == 'd') 
        {
            if (scanf("%d", &value) == 1) {
                int deleted = 0;
                root = delete(root, value, &deleted);
                printf("%s\n", deleted ? "deleted" : "absent");
            }
        } 
        else 
            break;
    }

    free_tree(root); //frees memory from the tree
    return 0;
}