#include <stdio.h>
#include <stdlib.h>


void multiply_matrices(int **result, int **A, int **B, int k) {
    int **temp = (int **)malloc(k * sizeof(int *));
    for (int i = 0; i < k; i++) {
        temp[i] = (int *)malloc(k * sizeof(int));
    }

    // Multiply A and B, store in temp
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            temp[i][j] = 0;
            for (int l = 0; l < k; l++) {
                temp[i][j] += A[i][l] * B[l][j];
            }
        }
    }

    // Copy temp to result
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            result[i][j] = temp[i][j];
        }
    }

    // Free temp matrix
    for (int i = 0; i < k; i++) {
        free(temp[i]);
    }
    free(temp);

}

void matrix_exponentiation(int **M, int k, int n) {
    // Allocates space for result matrix
    int **result = (int **)malloc(k * sizeof(int *));
    int **temp = (int **)malloc(k * sizeof(int *));
    for (int i = 0; i < k; i++) {
        result[i] = (int *)malloc(k * sizeof(int));
        temp[i] = (int *)malloc(k * sizeof(int));
    }

    // Initializes result as identity matrix

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            result[i][j] = (i == j) ? 1 : 0;
            temp[i][j] = M[i][j];
        }
    }

    // If n == 0, return identity matrix
    if (n == 0) {
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < k; j++) {
                printf("%d ", result[i][j]);
            }
            printf("\n");
        }
    } 
    
    else 
    {
        // Perform matrix exponentiation using repeated multiplication
        for (int i = 0; i < n; i++) {
            multiply_matrices(result, result, M, k);
        }

        // Print the final matrix

        for (int i = 0; i < k; i++) {

            for (int j = 0; j < k; j++) {

                printf("%d",result[i][j]);
                if (j < k - 1) 
                    printf(" ");

            }
            printf("\n");
        }
    }

    // Free allocated memory
    for (int i = 0; i < k; i++) {
        free(result[i]);
        free(temp[i]);
    }

    free(result);
    free(temp);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }
    // Open file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    int k;
    fscanf(file, "%d", &k); // Read matrix size

    // Allocate memory for matrix
    int **M = (int **)malloc(k * sizeof(int *));
    for (int i = 0; i < k; i++) {
        M[i] = (int *)malloc(k * sizeof(int));
    }

    // Read matrix values
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            fscanf(file, "%d", &M[i][j]);
        }
    }
    int n;
    fscanf(file, "%d", &n); // Read exponent

    // Close file
    fclose(file);

    // Perform matrix exponentiation
    matrix_exponentiation(M, k, n);

    // Free allocated memory
    for (int i = 0; i < k; i++) {
        free(M[i]);
    }
    free(M);
    return 0;
}