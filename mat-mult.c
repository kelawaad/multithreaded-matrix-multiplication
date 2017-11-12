#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int **A, **B, **C;
int x, y, z;

void nonThreadedMatMult(int **A, int **B, int **C, int x, int y, int z) {

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
}

void *calculateElement(void *param) {
	int *elements = (int*)param;
	int row = elements[0];
	int col = elements[1];
	printf("row = %d\tcol = %d\n", row, col);
	int sum = 0, i, j;
	for(i = 0; i < y;i++)
	{
		int mul = A[row][i] * B[i][col];
		printf("A[%d][%d] * B[%d][%d] = %d\n", row, i, i, col, mul); 
		sum += mul;
	}
	C[row][col] = sum;
	printf("Sum = %d\n", sum);
}

int threadedMatMultPerElement(int **A, int **B, int **C, int x, int y, int z) {
	int i = 0, j = 0;
	int count = 0;
	for(i = 0;i < x;i++) {
		for(j= 0;j < z;j++) {
			count++;
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
	return count;
}

int **createArray(int rows, int cols) {
	int **arr = malloc(rows * sizeof(int*));
	int i;
	for(i = 0;i < rows;i++)
		arr[i] = malloc(cols * sizeof(int));
	return arr;
}

void printArray(int **c, int x, int y) {
	int i, j;	
	for(i = 0;i < x;i++) {
		for(j = 0;j < y;j++) {
			printf("%d ", c[i][j]);
		}
		printf("\n");
	}
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
	int num_threads = threadedMatMultPerElement(A, B, C, x, y, z);
	printArray(C, x, z);
	printf("\n\nNumber of threads created: %d\n\n", num_threads);
	

}
