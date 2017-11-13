#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define DEBUG 1

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s():"fmt, file_name, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#undef DEBUG
#define DEBUG 0

/*
 * A: Input  matrix of size x*y
 * B: Input  matrix of size y*z
 * C: Output matrix of size x*z
 */
int **A, **B, **C;
int x, y, z;

/*
 * Calculates one element of the output array by
 * multiplying the corresponding row and column
 */
void *calculateElement(void *param) {
	int *elements = (int*)param;
	int row = elements[0]; // row of the element
	int col = elements[1]; // column of the element

	debug_print("row = %d\tcol = %d\n", row, col);

	int sum = 0, i, j;
	for(i = 0; i < y;i++)
	{
		int mul = A[row][i] * B[i][col];
		debug_print("A[%d][%d] * B[%d][%d] = %d\n", row, i, i, col, mul);
		sum += mul;
	}
	C[row][col] = sum;
	debug_print("sum = %d\n", sum);
	pthread_exit(0);
}

/*
 * Calculates one row of the output matrix
 * by multiplying the corresponding row with
 * all the columns of the other matrix
 */
void *calculateRow(void *param) {
	int row = ((int*)param)[0]; // row to be calculated
    int i, j;
    for(i = 0;i < y;i++) {
        int sum = 0
        for(j = 0; j < z; j++) {
            sum += A[row][j] * B[j][i];
        }
        C[row][i] = sum;
    }
    pthread_exit(0);
}

/*
 * Classic non-threaded matrix multiplication function
 * that runs in O(N^3)
 */
int nonThreadedMatMult() {
	int sum = 0;
	int i, j, k;
	for(i = 0;i < x;i++) {
		for(j = 0;j < z;j++) {
			sum = 0;
			for(k = 0;k < y;k++) {
				sum += A[i][k] * B[k][j];
			}
			C[i][j] = sum;
		}
	}
	return 1;
}

/*
 * Threaded matrix multiplication function, creates
 * 1 thread for each element in the output matrix.
 * returns number of created threads.
 */
int threadedMatMultPerElement() {
	int i = 0, j = 0;
	for(i = 0;i < x;i++) {
		for(j= 0;j < z;j++) {
			int *rowcol = malloc(2 * sizeof(int));
			rowcol[0] = i;
			rowcol[1] = j;
			pthread_t tid;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&tid, &attr, calculateElement, rowcol);
			pthread_join(tid, NULL);
		}
	}
	return x * z;
}

/*
 * Threaded matrix multiplication function, creates
 * 1 thread for each row in the output matrix.
 * returns number of created threads
 */
int threadedMatMultPerRow() {
	int i = 0;
	for(i = 0;i < x;i++) {
		int *row = malloc(sizeof(int));
		row[0] = i;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, calculateRow, row);
		pthread_join(tid, NULL);
	}
	return x;
}

int **createArray(int rows, int cols) {
	int **arr = malloc(rows * sizeof(int*));
	int i;
	for(i = 0;i < rows;i++)
		arr[i] = malloc(cols * sizeof(int));
	return arr;
}

void printArray(int **arr, int rows, int cols) {
	int i, j;
	for(i = 0;i < rows;i++) {
		for(j = 0;j < cols;j++) {
			printf("%d ", arr[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int main() {
	x = 3, y = 2, z = 3;
	int arr[] = {1, 2, 3, 4, 5, 6};
	A = createArray(x, y);
	B = createArray(y, z);

	int i, j;
	for(i = 0;i < x;i++)
		for(j = 0;j < y;j++)
			A[i][j] = arr[i * y + j];

	for(i = 0;i < y;i++)
		for(j = 0;j < z;j++)
			B[i][j] = arr[i * z + j];

	printArray(A, x, y);
	printArray(B, y, z);

	C = createArray(x, z);
	int num_threads = threadedMatMultPerElement();
	printArray(C, x, z);
	printf("\n\nNumber of threads created: %d\n\n", num_threads);
}
