#include <stdio.h>
#include <stdlib.h>

// Function to allocate memory for a matrix
double** allocateMatrix(int rows, int cols) {
    double** M = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++) {
        M[i] = (double*)malloc(cols * sizeof(double));
    }
    return M;
}

// Function to free allocated memory for a matrix
void freeMatrix(double** M, int rows) {
    for (int i = 0; i < rows; i++) {
        free(M[i]);
    }
    free(M);
}

// Function to transpose a matrix
double** transposeMatrix(double** M, int rows, int cols) {
    double** transposeM = allocateMatrix(cols, rows);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            transposeM[j][i] = M[i][j];
        }
    }
    return transposeM;
}

// Function to multiply two matrices
double** multiplyMatrices(double** A, int A_rows, int A_cols, double** B, int B_rows, int B_cols) {
    if (A_cols != B_rows) return NULL;
    double** result = allocateMatrix(A_rows, B_cols);
    for (int i = 0; i < A_rows; i++) {
        for (int j = 0; j < B_cols; j++) {
            result[i][j] = 0;
            for (int k = 0; k < A_cols; k++) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

// Function to invert a matrix using Gauss-Jordan elimination
int invertMatrix(double** M, int n) {
    double** I = allocateMatrix(n, n); //allocates memory for identity matrix
    for (int i = 0; i < n; i++) { //traverses row for matrix
        for (int j = 0; j < n; j++) { //traverses column for matrix
            I[i][j] = (i == j) ? 1.0 : 0.0; //sets up and initializes the inverse matrix to be the identity matrix.
        }
    }
    
    for (int p = 0; p < n; p++) {
        double f = M[p][p]; //pivot value
        if (f == 0) return 0; //if pivot is 0, return 0
        for (int j = 0; j < n; j++) { //traverses the matrix array of size n
            M[p][j] /= f; //divides the pivot row by the pivot value for the original matrix
            I[p][j] /= f; //divides the pivot row by the pivot vlue for the identity matrix
        }
        for (int i = 0; i < n; i++) {
            if (i != p) {
                f = M[i][p];
                for (int j = 0; j < n; j++) {
                    M[i][j] -= M[p][j] * f;
                    I[i][j] -= I[p][j] * f;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            M[i][j] = I[i][j];
        }
    }
    freeMatrix(I, n); //frees up allocated memory for the inverse matrix
    return 1; //returns 1 if the matrix is successfully inverted
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <train_file> <data_file>\n", argv[0]);
        return 1;
    }
    
    FILE* train_file = fopen(argv[1], "r"); //opens the training data file
    FILE* data_file = fopen(argv[2], "r"); //opens the input data file
    if (!train_file || !data_file) { //if either one of these files cannot be found, it returns an error
        printf("error\n");
        return 1;
    }
    
    int k, n;
    fscanf(train_file, "train\n%d\n%d\n", &k, &n); //k and n are the number of attributes and houses, respectively
    double** X = allocateMatrix(n, k + 1); //allocates memory for matrix X
    double** Y = allocateMatrix(n, 1); //allocates memory for matrix Y
    for (int i = 0; i < n; i++) {
        X[i][0] = 1.0; //sets the first column of matrix X to 1
        for (int j = 1; j <= k; j++) {
            fscanf(train_file, "%lf", &X[i][j]);
        }
        fscanf(train_file, "%lf", &Y[i][0]);
    }
    fclose(train_file);
    
    double** XT = transposeMatrix(X, n, k + 1); //takes the transpose of matrix X
    double** XTX = multiplyMatrices(XT, k + 1, n, X, n, k + 1); //multiplies matrix X with transpose of matrix X
    double** XTY = multiplyMatrices(XT, k + 1, n, Y, n, 1); //multiplies matrix Y with transpose of matrix X
    
    if (!invertMatrix(XTX, k + 1)) {
        printf("error\n");
        return 1;
    }
    
    double** W = multiplyMatrices(XTX, k + 1, k + 1, XTY, k + 1, 1); //calculates matrix W = (XTX)^-1 * XTY
    
    int m;
    fscanf(data_file, "data\n%d\n%d\n", &k, &m);
    double** X_new = allocateMatrix(m, k + 1);
    for (int i = 0; i < m; i++) {
        X_new[i][0] = 1.0;
        for (int j = 1; j <= k; j++) {
            fscanf(data_file, "%lf", &X_new[i][j]);
        }
    }
    fclose(data_file);
    
    double** Y_new = multiplyMatrices(X_new, m, k + 1, W, k + 1, 1);
    for (int i = 0; i < m; i++) {
        printf("%.0f\n", Y_new[i][0]);
    }
    
    freeMatrix(X, n);
    freeMatrix(Y, n);
    freeMatrix(XT, k + 1);
    freeMatrix(XTX, k + 1);
    freeMatrix(XTY, k + 1);
    freeMatrix(W, k + 1);
    freeMatrix(X_new, m);
    freeMatrix(Y_new, m);
    return 0;
}