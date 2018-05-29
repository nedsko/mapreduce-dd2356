#!/bin/bash -l
# The -l above is required to get the full environment with modules

#SBATCH -A edu18.DD2356

#SBATCH -J nedoMapReduce

# 1 hours wall-clock time will be given to this job
#SBATCH -t 01:00:00

# Number of nodes
#SBATCH --nodes=10
# Number of MPI processes per node
#SBATCH --ntasks-per-node=32

#SBATCH -e error_file.e

# Run 32 processes 5 times
aprun -n 32 -N 32 ./mapReduce_160.out
aprun -n 32 -N 32 ./mapReduce_160.out
aprun -n 32 -N 32 ./mapReduce_160.out
aprun -n 32 -N 32 ./mapReduce_160.out
aprun -n 32 -N 32 ./mapReduce_160.out

# Run 64 processes 5 times
aprun -n 64 -N 32 ./mapReduce_160.out
aprun -n 64 -N 32 ./mapReduce_160.out
aprun -n 64 -N 32 ./mapReduce_160.out
aprun -n 64 -N 32 ./mapReduce_160.out
aprun -n 64 -N 32 ./mapReduce_160.out

# Run 96 processes 5 times
aprun -n 96 -N 32 ./mapReduce_160.out
aprun -n 96 -N 32 ./mapReduce_160.out
aprun -n 96 -N 32 ./mapReduce_160.out
aprun -n 96 -N 32 ./mapReduce_160.out
aprun -n 96 -N 32 ./mapReduce_160.out

# Run 128 processes 5 times
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out
aprun -n 128 -N 32 ./mapReduce_160.out

# Run 160 processes 5 times
aprun -n 160 -N 32 ./mapReduce_160.out
aprun -n 160 -N 32 ./mapReduce_160.out
aprun -n 160 -N 32 ./mapReduce_160.out
aprun -n 160 -N 32 ./mapReduce_160.out
aprun -n 160 -N 32 ./mapReduce_160.out

# Run 192 processes 5 times
aprun -n 192 -N 32 ./mapReduce_160.out
aprun -n 192 -N 32 ./mapReduce_160.out
aprun -n 192 -N 32 ./mapReduce_160.out
aprun -n 192 -N 32 ./mapReduce_160.out
aprun -n 192 -N 32 ./mapReduce_160.out

# Run 224 processes 5 times
aprun -n 224 -N 32 ./mapReduce_160.out
aprun -n 224 -N 32 ./mapReduce_160.out
aprun -n 224 -N 32 ./mapReduce_160.out
aprun -n 224 -N 32 ./mapReduce_160.out
aprun -n 224 -N 32 ./mapReduce_160.out

# Run 256 processes 5 times
aprun -n 256 -N 32 ./mapReduce_160.out
aprun -n 256 -N 32 ./mapReduce_160.out
aprun -n 256 -N 32 ./mapReduce_160.out
aprun -n 256 -N 32 ./mapReduce_160.out
aprun -n 256 -N 32 ./mapReduce_160.out

# Run 288 processes 5 times
aprun -n 288 -N 32 ./mapReduce_160.out
aprun -n 288 -N 32 ./mapReduce_160.out
aprun -n 288 -N 32 ./mapReduce_160.out
aprun -n 288 -N 32 ./mapReduce_160.out
aprun -n 288 -N 32 ./mapReduce_160.out

# Run 320 processes 5 times
aprun -n 320 -N 32 ./mapReduce_160.out
aprun -n 320 -N 32 ./mapReduce_160.out
aprun -n 320 -N 32 ./mapReduce_160.out
aprun -n 320 -N 32 ./mapReduce_160.out
aprun -n 320 -N 32 ./mapReduce_160.out
