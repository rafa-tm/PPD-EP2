#!/bin/bash
#SBATCH -J ep5_ppd               # Job name
#SBATCH -p fast                  # Job partition
#SBATCH -n 1                     # Number of processes
#SBATCH -t 01:30:00              # Run time (hh:mm:ss)
#SBATCH --cpus-per-task=128      # Number of CPUs per process
#SBATCH --output=%x.out          # Name of stdout output file - %j expands to jobId and %x to jobName
#SBATCH --error=%x.err           # Name of stderr output file

lscpu
echo "    "
echo "*** SEQUENCIAL ***"
srun singularity run container.sif wave_seq 500 500 5000

echo "*** OPENMP ***"
for num_threads in {1,2,5,10,20,40,64}
do
    srun singularity run container.sif wave_omp 500 500 5000 $num_threads
done