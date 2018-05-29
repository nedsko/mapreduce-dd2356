#!/bin/bash -l
# The -l above is required to get the full environment with modules

#SBATCH -A edu18.DD2356

#SBATCH -J nedoMapReduce

# 1 hours wall-clock time will be given to this job
#SBATCH -t 10:00:00

# Number of nodes
#SBATCH --nodes=10
# Number of MPI processes per node
#SBATCH --ntasks-per-node=32

#SBATCH -e error_file.e

# Run 8 processes 5 times with 10 GB
aprun -n 8 -N 8 ./mapReduce_10.out
aprun -n 8 -N 8 ./mapReduce_10.out
aprun -n 8 -N 8 ./mapReduce_10.out
aprun -n 8 -N 8 ./mapReduce_10.out
aprun -n 8 -N 8 ./mapReduce_10.out

# Run 16 processes 5 times with 20 GB
aprun -n 16 -N 16 ./mapReduce_20.out
aprun -n 16 -N 16 ./mapReduce_20.out
aprun -n 16 -N 16 ./mapReduce_20.out
aprun -n 16 -N 16 ./mapReduce_20.out
aprun -n 16 -N 16 ./mapReduce_20.out

# Run 32 processes 5 times with 40 GB
aprun -n 32 -N 32 ./mapReduce_40.out
aprun -n 32 -N 32 ./mapReduce_40.out
aprun -n 32 -N 32 ./mapReduce_40.out
aprun -n 32 -N 32 ./mapReduce_40.out
aprun -n 32 -N 32 ./mapReduce_40.out

# Run 64 processes 5 times with 80 GB
aprun -n 64 -N 32 ./mapReduce_80.out
aprun -n 64 -N 32 ./mapReduce_80.out
aprun -n 64 -N 32 ./mapReduce_80.out
aprun -n 64 -N 32 ./mapReduce_80.out
aprun -n 64 -N 32 ./mapReduce_80.out

# Run 128 processes 5 times with 160 GB
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out

# Run 256 processes 5 times with 320 GB
aprun -n 256 -N 32 ./mapReduce_320.out
aprun -n 256 -N 32 ./mapReduce_320.out
aprun -n 256 -N 32 ./mapReduce_320.out
aprun -n 256 -N 32 ./mapReduce_320.out
aprun -n 256 -N 32 ./mapReduce_320.out
