# the compiler: gcc for C program, define as g++ for C++
CC = mpicxx

 # the build target executable:
TARGET = mapReduse

all:
	$(CC) $(TARGET).cpp -o $(TARGET) -std=c++11

run:
	mpirun -n 2 ./$(TARGET)
