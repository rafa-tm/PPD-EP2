#!/bin/bash
#SBATCH -J ep5_ppd               # Job name
#SBATCH -p fast                  # Job partition
#SBATCH -n 1                     # Number of processes
#SBATCH -t 01:30:00              # Run time (hh:mm:ss)
#SBATCH --cpus-per-task=96      # Number of CPUs per process
#SBATCH --output=%x.out          # Name of stdout output file - %j expands to jobId and %x to jobName
#SBATCH --error=%x.err           # Name of stderr output file

N1=500
N2=500
TIME=5000

lscpu
echo "    "
echo "*** SEQUENCIAL ***"
srun singularity run container.sif wave_seq $N1 $N2 $TIME

echo "*** OPENMP ***"
for num_threads in {1,2,5,10,20,40,64}
do
    srun singularity run container.sif wave_omp $N1 $N2 $TIME $num_threads
done