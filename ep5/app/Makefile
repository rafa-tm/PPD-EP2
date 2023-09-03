CC=gcc

all: clean sequential

sequential:
	$(CC) wave_seq.c -o wave_seq

omp:
	$(CC) wave_omp.c -o wave_omp -fopenmp

clean:
	rm -f wave_seq wave_omp wave_mpi wave_pth wave_cuda
