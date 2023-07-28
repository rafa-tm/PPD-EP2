#!/bin/bash
#SBATCH -J laplace_ppd                   # Job name
#SBATCH -p fast                     # Job partition
#SBATCH -n 1                        # Number of processes
#SBATCH -t 01:30:00                 # Run time (hh:mm:ss)
#SBATCH --cpus-per-task=40          # Number of CPUs per process
#SBATCH --output=%x.out          # Name of stdout output file - %j expands to jobId and %x to jobName
#SBATCH --error=%x.err           # Name of stderr output file

lscpu
echo "    "
for i in {512,1024,2048};
 do
    echo "    "
    echo "*** SEQUENTIAL ***"
    srun singularity run container.sif laplace_seq $i
    for j in {1,2,5,10,20,40};
        do
            echo "*** PTHREAD COM $j THREADS ***"
            srun singularity run container.sif laplace_pth $i $j
            echo "    "
        done
    echo "    "
 done