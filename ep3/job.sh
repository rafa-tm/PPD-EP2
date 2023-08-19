#!/bin/bash
#SBATCH -J lap_opm                   # Job name
#SBATCH --partition=fast             # Partition (queue) name
#SBATCH -n 1                        # Number of processes
#SBATCH -t 01:30:00                 # Run time (hh:mm:ss)
#SBATCH --cpus-per-task=96          # Number of CPUs per task
#SBATCH --output=%x.out          # Name of stdout output file - %j expands to jobId and %x to jobName
#SBATCH --error=%x.err           # Name of stderr output file

    lscpu
    echo "    "
    echo "*** SEQUENTIAL ***"
    srun singularity run container.sif laplace_seq 2048
    for j in {1,2,5,10,20,40,64};
        do
            export OMP_NUM_THREADS=$j
            echo "*** OPENMP COM $j THREADS ***"
            srun singularity run container.sif laplace_omp 2048
            echo "    "
            echo "*** OPENMP COLLAPSE COM $j THREADS ***"
            srun singularity run container.sif laplace_omp_collapse 2048
            echo "    "
        done
    echo "    "
 