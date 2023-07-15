CC=gcc

all: clean sequential openmp openmp2

sequential:
	$(CC) laplace_seq.c -o laplace_seq

openmp:
	$(CC) laplace_omp.c -o laplace_omp -fopenmp
	#/usr/local/opt/llvm/bin/clang pi_omp.c -o pi_omp -fopenmp -L/usr/local/opt/llvm/lib

openmp2:
	$(CC) laplace_omp_collapse.c -o laplace_omp_collapse -fopenmp
	#/usr/local/opt/llvm/bin/clang pi_omp.c -o pi_omp -fopenmp -L/usr/local/opt/llvm/lib

clean:
	rm -f laplace_seq laplace_omp laplace_omp_collapse
