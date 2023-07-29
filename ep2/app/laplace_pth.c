#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define ITER_MAX 3000 // number of maximum iterations
#define CONV_THRESHOLD 1.0e-5f // threshold of convergence

// matrix to be solved
double **grid;

// auxiliary matrix
double **new_grid;

// size of each side of the grid
int size;

// number of threads
int num_threads;

// return the maximum value
double max(double a, double b){
    if(a > b)
        return a;
    return b;
}

// return the absolute value of a number
double absolute(double num){
    if(num < 0)
        return -1.0 * num;
    return num;
}

// allocate memory for the grid
void allocate_memory(){
    grid = (double **) malloc(size * sizeof(double *));
    new_grid = (double **) malloc(size * sizeof(double *));

    for(int i = 0; i < size; i++){
        grid[i] = (double *) malloc(size * sizeof(double));
        new_grid[i] = (double *) malloc(size * sizeof(double));
    }
}

// initialize the grid
void initialize_grid(){
    // seed for random generator
    srand(10);

    int linf = size / 2;
    int lsup = linf + size / 10;
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            // initialize region of heat in the center of the grid
            if (i >= linf && i < lsup && j >= linf && j < lsup)
                grid[i][j] = 100;
            else
               grid[i][j] = 0;
            new_grid[i][j] = 0.0;
        }
    }
}

// save the grid in a file
void save_grid(){
    char file_name[30];
    sprintf(file_name, "grid_pth.txt");

    // save the result
    FILE *file;
    file = fopen(file_name, "w");

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            fprintf(file, "%lf ", grid[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

pthread_barrier_t barrier;

void *calc_laplace_parallel(void *);

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: ./laplace_seq N T\n");
        printf("N: The size of each side of the domain (grid)\n");
        printf("T: The number of threads\n");
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

    // create an array of pthreads
    pthread_t threads[num_threads];
   
    // store each thread ID
    int t_id[num_threads];

    int iter = 0;

    printf("Jacobi relaxation calculation: %d x %d grid\n", size, size);

    // get the start time
    gettimeofday(&time_start, NULL);

    // initialize the barrier
    pthread_barrier_init(&barrier, NULL, num_threads);
    
    // create the threads
    for(int i = 0; i < num_threads; i++){
        t_id[i] = i;
        pthread_create(&threads[i], NULL, calc_laplace_parallel, (void *) &t_id[i]);
    }

    // wait for the threads to finish
    for(int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    // save the final grid in a file
    save_grid();

    printf("\nExecutado em %lf segundos\n", exec_time);
    return 0;
}

void *calc_laplace_parallel(void *args){
    // thread id
    int id = *(int *) args;
    
    // calculate start and end step of the thread
    int start = id * (size / num_threads);
    int end = start + (size / num_threads);

    // handle the remaining elements for the last thread
    if (id == num_threads - 1)
        end = size - 1;

    int iter = 0;
    double diff, max_diff;

    while (iter <= ITER_MAX) {
        // Boundary condition update (top and bottom rows)
        if (id == 0) {
            for (int j = 1; j < size - 1; j++) {
                new_grid[0][j] = 0.25 * (grid[0][j + 1] + grid[0][j - 1] + grid[1][j] + grid[size - 1][j]);
            }
        }

        if (id == num_threads - 1) {
            for (int j = 1; j < size - 1; j++) {
                new_grid[size - 1][j] = 0.25 * (grid[size - 1][j + 1] + grid[size - 1][j - 1] + grid[0][j] + grid[size - 2][j]);
            }
        }

        // Jacobi iteration
        for (int i = start + 1; i <= end - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                new_grid[i][j] = 0.25 * (grid[i][j + 1] + grid[i][j - 1] + grid[i - 1][j] + grid[i + 1][j]);
            }
        }

        // Synchronize all threads before updating the grid
        pthread_barrier_wait(&barrier);

        // Calculate the maximum difference between the grids
        max_diff = 0.0;
        for (int i = start + 1; i <= end - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                diff = absolute(new_grid[i][j] - grid[i][j]);
                max_diff = max(max_diff, diff);
            }
        }

        // Synchronize all threads before updating the grid and checking convergence
        pthread_barrier_wait(&barrier);

        // Copy the new_grid to grid for the next iteration
        for (int i = start + 1; i <= end - 1; i++) {
            for (int j = 1; j < size - 1; j++) {
                grid[i][j] = new_grid[i][j];
            }
        }

        // Check convergence
        if (max_diff < CONV_THRESHOLD) {
            break;
        }

        iter++;
    }

    pthread_exit(NULL);
}
