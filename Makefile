# the compiler: gcc for C program, define as g++ for C++
CC = mpicxx

path = /home/c/mhpc/mapReduse

 # the build target executable:
TARGET = mapReduse

all: func.o
	$(CC) $(TARGET).cpp -o $(TARGET).out func.o -std=c++11


func.o: func.cpp func.hpp
	g++ -I$(shell pwd) -c func.cpp

run:
	make
	clear
	mpirun -n 4 ./$(TARGET).out

test: func.o
	g++ -I$(shell pwd) test.cpp -o test.out func.o
