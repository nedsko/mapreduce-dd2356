#include <mpi.h>
#include <iostream>
#include <string>

#define FILE "wikipedia_test_small.txt"
#define MASTER 0
using namespace std;



int main(int argc, char *argv[]){
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_File fh;

	char *send_buf = new char[64*size-1];
	char *re_buf = new char[64];

	int nr_of_reads;
	if(rank == MASTER){
		MPI_File_open( MPI_COMM_SELF , FILE, MPI_MODE_RDONLY , MPI_INFO_NULL , &fh);
		MPI_Offset file_size;
		MPI_File_get_size(fh, &file_size);
		cout<<"file_size"<<file_size<<endl;
		nr_of_reads = (double)file_size/(double)(64*size);
		
	}
	MPI_Bcast(&nr_of_reads, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	
	
	
	cout<<"nr of reads "<<nr_of_reads<<endl;
	for(int i = 0; i<nr_of_reads;i++){

		if(rank == MASTER){
			MPI_File_read(fh,send_buf, 64*size-1,MPI_BYTE, MPI_STATUS_IGNORE);
		}

		MPI_Scatter(send_buf,64,MPI_BYTE,re_buf,64,MPI_BYTE,MASTER,MPI_COMM_WORLD);
		if(rank!=MASTER){
			//do slave work
			cout<<"rank"<<rank;
			for(int j = 0; j<64; j++){
				cout<<re_buf[j];
			}
			cout<<endl;
		}
		


	}

	
		
	
	
	
	//cout<<"FILE"<<file_size<<endl;
	//cout<<"print"<<endl;
	//for(int i = 0; i<64; i++){
	//	cout<<buf[i]<<endl;
	//}
	MPI_Finalize();
	return 0;
}