#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define ITER_MAX 3000 // number of maximum iterations
#define CONV_THRESHOLD 1.0e-5f // threshold of convergence

// matrix to be solved
double **grid;
double **new_grid;

// size of each side of the grid
int size;

// number of threads to use
int num_threads;

// return the maximum value
double max(double a, double b) {
    if (a > b)
        return a;
    return b;
}

// return the absolute value of a number
double absolute(double num) {
    if (num < 0)
        return -1.0 * num;
    return num;
}

// allocate memory for the grid
void allocate_memory() {
    grid = (double **)malloc(size * sizeof(double *));
    new_grid = (double **)malloc(size * sizeof(double *));

    for (int i = 0; i < size; i++) {
        grid[i] = (double *)malloc(size * sizeof(double));
        new_grid[i] = (double *)malloc(size * sizeof(double));
    }
}

// initialize the grid
void initialize_grid() {
    // seed for random generator
    srand(10);

    int linf = size / 2;
    int lsup = linf + size / 10;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // initialize region of heat in the center of the grid
            if (i >= linf && i < lsup && j >= linf && j < lsup)
                grid[i][j] = 100;
            else
                grid[i][j] = 0;
            new_grid[i][j] = 0.0;
        }
    }
}

// pthread function to calculate the Jacobi iteration for a portion of the grid
// pthread function to calculate the Jacobi iteration for a portion of the grid
void* calculate_jacobi(void* arg) {
    int thread_id = *(int*)arg;
    int chunk_size = size / num_threads;
    int start_row = thread_id * chunk_size;
    int end_row = (thread_id == num_threads - 1) ? size - 1 : (thread_id + 1) * chunk_size;

    double* local_err_ptr = (double*)malloc(sizeof(double));
    *local_err_ptr = 0.0;

    // calculates the Laplace equation to determine each cell's next value
    for (int i = start_row + 1; i < end_row; i++) {
        for (int j = 1; j < size - 1; j++) {
            new_grid[i][j] = 0.25 * (grid[i][j + 1] + grid[i][j - 1] +
                                     grid[i - 1][j] + grid[i + 1][j]);

            *local_err_ptr = max(*local_err_ptr, absolute(new_grid[i][j] - grid[i][j]));
        }
    }

    pthread_exit(local_err_ptr);
}


// save the grid in a file
void save_grid() {
    char file_name[30];
    sprintf(file_name, "grid_laplace.txt");

    // save the result
    FILE *file;
    file = fopen(file_name, "w");

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%lf ", grid[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: ./laplace_pthread N num_threads\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("num_threads: Number of threads to use\n");
        exit(-1);
    }

    // variables to measure execution time
    struct timeval time_start;
    struct timeval time_end;

    size = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    // allocate memory to the grid (matrix)
    allocate_memory();

    // set grid initial conditions
    initialize_grid();

    double err = 1.0;
    int iter = 0;

    printf("Jacobi relaxation calculation: %d x %d grid\n", size, size);

    // get the start time
    gettimeofday(&time_start, NULL);

    // Jacobi iteration
    // This loop will end if either the maximum change reaches below a set threshold (convergence)
    // or a fixed number of maximum iterations have completed
    while (err > CONV_THRESHOLD && iter <= ITER_MAX) {

        err = 0.0;
        pthread_t threads[num_threads];
        int thread_ids[num_threads];

        // Create threads
        for (int t = 0; t < num_threads; t++) {
            thread_ids[t] = t;
            pthread_create(&threads[t], NULL, calculate_jacobi, &thread_ids[t]);
        }

        // Join threads and collect local error values
        for (int t = 0; t < num_threads; t++) {
            double* local_err_ptr;
            pthread_join(threads[t], (void**)&local_err_ptr);
            err = max(err, *local_err_ptr);
            free(local_err_ptr); // Free the memory allocated in the thread function
        }

        // copy the next values into the working array for the next iteration
        for (int i = 1; i < size - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                grid[i][j] = new_grid[i][j];
            }
        }

        

        iter++;
    }

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double)(time_end.tv_sec - time_start.tv_sec) +
                       (double)(time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    // save the final grid in file
    save_grid();

    printf("\nKernel executed in %lf seconds with %d iterations \n", exec_time, iter);

    return 0;
}
