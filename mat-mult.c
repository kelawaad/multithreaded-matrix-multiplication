#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>



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
	for(i = 0;i < x;i++) {
		for(j= 0;j < z;j++) {
			int *rowcol = (int*)malloc(2 * sizeof(int));
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
		int *row = (int*)malloc(sizeof(int));
		row[0] = i;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, calculateRow, row);
		pthread_join(tid, NULL);
	}
	return x;
}

double **createArray(int rows, int cols) {
	double **arr = (double**)malloc(rows * sizeof(double*));
	int i;
	for(i = 0;i < rows;i++)
		arr[i] = (double*)malloc(cols * sizeof(double));
	return arr;
}

double **createRandomArray(int rows, int cols) {
    double **arr = createArray(rows, cols);
    int i, j;
    for(i = 0; i < rows; i++)
        for(j = 0;j < cols; j++)
            arr[i][j] = rand() % 10;
    return arr;
}

void freeArray(double **arr, int rows) {
    int i;
    for(i = 0; i < rows; i++)
        free(arr[i]);
    free(arr);
}

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

double **readArrayFromFile(const char *fname, int *rows_in, int *cols_in) {
	FILE *fp = fopen(fname, "r");

	int rows = 0, cols = 0;
	char c;
	do {
		c = fgetc(fp);
		if(c == ' ' && rows == 0) {
			cols++;
			continue;
		}
		if(c == '\n') {
			if(rows == 0)
				cols++;
			*cols_in = cols;
			rows++;
			continue;
		}
		if(c == EOF)
			break;
	}while(1);
	rewind(fp);
	*rows_in = rows;

	double **arr = createArray(rows, cols);
	rows = cols = 0;
	do {
		fscanf(fp, "%lf", &arr[rows][cols]);
		c = fgetc(fp);
		if(c == ' ') {
			cols++;
			continue;
		}
		if(c == '\n') {
			cols = 0;
			rows++;
			continue;
		}
		if(c == EOF)
			break;
	}while(1);
	return arr;
}

void printArrayToFile(double **arr, int rows, int cols, char *name) {
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
	fp funcs[3];
	funcs[0] = nonThreadedMatMult;
	funcs[1] = threadedMatMultPerElement;
	funcs[2] = threadedMatMultPerRow; 
	printf("pid = %d\n", getpid());
	if(argc > 2 && strcmp(argv[1], "-b") != 0) {
		int y_copy;
		A = readArrayFromFile(argv[1], &x, &y);
		B = readArrayFromFile(argv[2], &y_copy, &z);
		debug_print("x = %d\ty = %d\tz = %d\n", x, y, z);
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
		startTime = (float)clock()/CLOCKS_PER_SEC;
		num_threads = funcs[choice - 1]();
		endTime = (float)clock()/CLOCKS_PER_SEC;
		printf("\n");
		printArray(C, x, z);
		printf("\n");
		printf("Time elapsed: %.6lf secs\n", endTime - startTime);
		printf("Number of threads created: %d\n", num_threads);
		printArrayToFile(C, x, z, get_file_name(C));
		
		return 0;
	}
	else {
		int iterations;
		if(argc > 2 && strcmp("-b", argv[1]) == 0)
			iterations = atoi(argv[2]);
		else if(argc == 1)
			iterations = 5;
		else
		{
			printf("Incorrect usage!\n");
			return 0;
		}
		
		FILE *fp = fopen("benchmark.txt", "w");
		int n = 1;
		x = y = z = n;
		double startTime, endTime, timeElapsed;
		time_t t;
		srand((unsigned int)t);
		file_name = getFileName(__FILE__);
		int i;

		for(i = 0;i < iterations;i++) {
			n = 2 * n;		
			x = y = z = n;
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
		freeArray(A, n);
		freeArray(B, n);
		freeArray(C, n);
		fclose(fp);
	}
	return 0;
}
