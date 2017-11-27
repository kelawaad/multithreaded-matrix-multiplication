#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>


// Used for debugging purposes
#define DEBUG 1

#define get_file_name(varname) #varname".txt"
#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s():"fmt, file_name, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#undef DEBUG
#define DEBUG 0

#ifdef _WIN32
    #define PATH_SEPARATOR '\\'
#elif __linux__
    #define PATH_SEPARATOR '/'
#endif

int flag = 0;

// Define a pointer to function named fp
typedef int (*fp)(void);

// Gets the name of the source file
char* getFileName(char *file_name_wpath)
{
    int length = strlen(file_name_wpath);
    int index = length - 1;
    while(index >= 0 && file_name_wpath[index] != PATH_SEPARATOR)
        index--;

    char *file_name = (char*)malloc((length - index + 1) * sizeof(char));
    int i = index + 1, j = 0;

    // Copy the filename into the filename variable
    for(;i < length;)
        file_name[j++] = file_name_wpath[i++];

    file_name[j] = 0; // Add '\0' to the end of the string
    return file_name;
}

char *file_name;

/*
 * A: Input  matrix of size x*y
 * B: Input  matrix of size y*z
 * C: Output matrix of size x*z
 */
double **A, **B, **C;
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

	int i, j;
	double sum = 0;
	for(i = 0; i < y;i++)
	{
		double mul = A[row][i] * B[i][col];
		debug_print("A[%d][%d] * B[%d][%d] = %lf\n", row, i, i, col, mul);
		sum += mul;
	}
	C[row][col] = sum;
	debug_print("sum = %lf\n", sum);
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

    for(i = 0;i < z;i++) {
        double sum = 0;
        for(j = 0; j < y; j++) {
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
	double sum = 0;
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
	pthread_t **threads_arr = (pthread_t**)malloc(x * sizeof(pthread_t*));
	for(i = 0;i < x;i++) {
		threads_arr[i] = (pthread_t*)malloc(z * sizeof(pthread_t));
		for(j= 0;j < z;j++) {
			int *rowcol = (int*)malloc(2 * sizeof(int));
			rowcol[0] = i;
			rowcol[1] = j;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&threads_arr[i][j], &attr, calculateElement, rowcol);
		}
	}
	for(i = 0;i < x;i++)
		for(j = 0;j < z;j++)
			pthread_join(threads_arr[i][j], NULL);
	return x * z;
}

/*
 * Threaded matrix multiplication function, creates
 * 1 thread for each row in the output matrix.
 * returns number of created threads
 */
int threadedMatMultPerRow() {
	pthreads_t *pthreads_arr = malloc(x * sizeof(pthread_t));
	int i = 0;
	for(i = 0;i < x;i++) {
		int *row = (int*)malloc(sizeof(int));
		row[0] = i;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&pthreads_arr[i], &attr, calculateRow, row);
	}
	for(i = 0;i < x; i++)
		pthread_join(pthreads_arr[i], NULL);
	return x;
}

/*
 * Creates and returns a matrix of dimensions rows * cols
 */ 
double **createArray(int rows, int cols) {
	double **arr = (double**)malloc(rows * sizeof(double*));
	int i;
	for(i = 0;i < rows;i++)
		arr[i] = (double*)malloc(cols * sizeof(double));
	return arr;
}

/*
 * Creates and returns a matrix of dimensions rows * cols
 * filled with random elements
 */ 
double **createRandomArray(int rows, int cols) {
    double **arr = createArray(rows, cols);
    int i, j;
    for(i = 0; i < rows; i++)
        for(j = 0;j < cols; j++)
            arr[i][j] = rand() % 10;
    return arr;
}

/*
 * Used to free a matrix that "rows" rows
 */
void freeArray(double **arr, int rows) {
	if(arr == NULL)
		return;
    int i;
    for(i = 0; i < rows; i++)
        free(arr[i]);
    free(arr);
}

/*
 * Prints a matrix of size rows * cols
 */ 
void printArray(double **arr, int rows, int cols) {
	int i, j;
	for(i = 0;i < rows;i++) {
		for(j = 0;j < cols;j++) {
			printf("%.2lf ", arr[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

/*
 * Reads and returns a matrix from a file, given by fname
 * returns the number of rows and columns as well.
 */
double **readArrayFromFile(const char *fname, int *rows_in, int *cols_in) {
	FILE *fp = fopen(fname, "r");

	int rows = 0, cols = 0;
	char c;
	// Get the dimensions of the matrix
	do {
		c = fgetc(fp);
		if(c == ' ' && rows == 0) {
			cols++;
			continue;
		}
		// If end of row
		if(c == 13) {
			do{
				c=fgetc(fp);
			}while(c == '\r');
			ungetc(c, fp);
			if(rows == 0)
				cols++;
			*cols_in = cols;
			rows++;
			continue;
		}
		// If end of file
		if(c == EOF) {
			rows++;
			break;
		}
	}while(1);
	// rewind the pointer to the beginning of the file
	rewind(fp);
	*rows_in = rows;

	printf("rows = %d\tcols = %d", rows, cols);

	// Create a matrix
	double **arr = createArray(rows, cols);
	rows = cols = 0;
	// fill in the matrix
	do {
		fscanf(fp, "%lf", &arr[rows][cols]);
		
		c = fgetc(fp);
		//if(flag==0)
		//printf("rows = %d cols = %d c = %d\n",rows, cols, c);
		if(c == ' ') {
			cols++;
			continue;
		}
		if(c == '\r') {
			do{
				c=fgetc(fp);
			}while(c == '\r');
			ungetc(c, fp);
			cols = 0;
			rows++;
			continue;
		}
		if(c == EOF)
			break;
	}while(1);
	return arr;
}

/*
 * Prints a 2-D matrix of size rows * cols to a file
 */ 
void printArrayToFile(double **arr, int rows, int cols, const char *name) {
	FILE *fp = fopen(name, "w");
	int i, j;
	for(i = 0;i < rows;i++) {
		for(j = 0;j < cols;j++)
			fprintf(fp, "%.5lf ", arr[i][j]);
		fprintf(fp, "\n");
	}
}

int main(int argc, char *argv[]) {
	A = B = C = NULL;

	// An array of pointers to function
	fp funcs[3];
	funcs[0] = nonThreadedMatMult;
	funcs[1] = threadedMatMultPerElement;
	funcs[2] = threadedMatMultPerRow; 
	
	if(argc > 2 && strcmp(argv[1], "-i") != 0 && strcmp(argv[1], "-n") != 0) {
		int y_copy;
		// Read the 2 matrices from the given files
		A = readArrayFromFile(argv[1], &x, &y);
		B = readArrayFromFile(argv[2], &y_copy, &z);
		
		printArray(A,x,y);
		printf("=======================================\n\n\n\n\n\n\n\n\n\n");
		printArray(B,y,z);

		printf("x = %d\ty = %d\tz = %d\n", x, y, z);
		debug_print("x = %d\ty = %d\tz = %d\n", x, y, z);

		// Check that the dimensions match
		if(y_copy != y) {
			printf("x = %d\ty = %d\ty_copy = %d\tz = %d\n", x, y, y_copy, z);
		 	printf("Cannot multiply the 2 matrices");
		 	exit(0);
		}
		
		int choice;
		printf("Which type of function do you wish to apply:\n1- Non threaded\n2- Thread per element\n3- Thread per row\n");
		scanf("%d", &choice);

		double startTime, endTime, timeElapsed;
		int num_threads;
		time_t t;
		
		C = createArray(x, z);
		
		// Call the chosen function and calculate the time taken
		startTime = (float)clock()/CLOCKS_PER_SEC;
		num_threads = funcs[choice - 1]();
		endTime = (float)clock()/CLOCKS_PER_SEC;
		
		printf("\n");
		printArray(C, x, z);
		printf("\n");

		printf("Time elapsed: %.6lf secs\n", endTime - startTime);
		printf("Number of threads created: %d\n", num_threads);
		
		// Save the matrix to the corresponding file
		printArrayToFile(C, x, z, get_file_name(C));
		
		return 0;
	}
	else {
		int iterations, n;
		if(argc > 2 && strcmp("-i", argv[1]) == 0)
		{
			iterations = atoi(argv[2]);
			if(argc > 4 && strcmp("-n", argv[3]) == 0)
				n = atoi(argv[4]);
			else
				n = 2;
		}
		else if(argc > 2 && strcmp("-n", argv[1]) == 0)
		{
			n = atoi(argv[2]) / 2;
			if(argc > 4 && strcmp("-i", argv[3]) == 0)
				iterations = atoi(argv[4]);
			else
			iterations = 5;
		}
		else if(argc == 1) {
			iterations = 5;
			n = 2;
		}
		else
		{
			printf("Incorrect usage!\n");
			return 0;
		}
		
		FILE *fp = fopen("benchmark2.txt", "w");
		x = y = z = n;
		double startTime, endTime, timeElapsed;
		time_t t;
		srand((unsigned int)t);
		file_name = getFileName(__FILE__);
		int i, old_n = n;
		for(i = 0;i < iterations;i++, n *= 2) {
			
			x = y = z = n;

			// Free the currently allocated matrices
			freeArray(A, old_n);
			freeArray(B, old_n);
			freeArray(C, old_n);
			old_n = n;
			
			// Allocate memory for the matrices with the new dimension
			A = createRandomArray(n, n);
			B = createRandomArray(n, n);
			C = createArray(n, n);

			printf("Number of elements: %d\n\n", n * n);

			startTime = (float)clock()/CLOCKS_PER_SEC;
			int num_threads = nonThreadedMatMult();
			endTime = (float)clock()/CLOCKS_PER_SEC;
			double timeElapsed1 = endTime - startTime;			
			printf("Number of threads created: %d\nTime Elapsed: %lf\n", num_threads, timeElapsed1);

			startTime = (float)clock()/CLOCKS_PER_SEC;
			num_threads = threadedMatMultPerElement();
			endTime = (float)clock()/CLOCKS_PER_SEC;
			double timeElapsed2 = endTime - startTime;
			printf("\nNumber of threads created: %d\nTime Elapsed: %lf\n", num_threads, timeElapsed2);
			
			startTime = (float)clock()/CLOCKS_PER_SEC;
			num_threads = threadedMatMultPerRow();
			endTime = (float)clock()/CLOCKS_PER_SEC;
			double timeElapsed3 = endTime - startTime;
			printf("\nNumber of threads created: %d\nTime Elapsed: %lf\n", num_threads, timeElapsed3);
			
			printf("============================================================================================\n");
			
			fprintf(fp, "%d,%.8lf,%.8lf,%0.8f\n", n, timeElapsed1, timeElapsed2, timeElapsed3);

			
		}
		freeArray(A, old_n);
		freeArray(B, old_n);
		freeArray(C, old_n);
		fclose(fp);
	}
	return 0;
}
