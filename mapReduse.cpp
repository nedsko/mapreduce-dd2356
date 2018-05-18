#include <mpi.h>
#include <iostream>
#include <string>
#include "func.hpp"
#include <vector>
#include <map>
#include <cstdint>
#define FILE "number_test.txt"
#define MASTER 0



using namespace std;


#define SEED_LENGTH 65

const char key_seed[SEED_LENGTH] = "b4967483cf3fa84a3a233208c129471ebc49bdd3176c8fb7a2c50720eb349461";
const unsigned short *key_seed_num = (unsigned short*)key_seed;

int calculateDestRank(const char *word, int length, int num_ranks)
{
    uint64_t hash = 0;
    
    for (uint64_t i = 0; i < length; i++)
    {
        uint64_t num_char = (uint64_t)word[i];
        uint64_t seed     = (uint64_t)key_seed_num[(i % SEED_LENGTH)];
        
        hash += num_char * seed * (i + 1);
    }
    
    return (int)(hash % (uint64_t)num_ranks);
}



int main(int argc, char *argv[]){
	int rank, num_ranks;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

	MPI_File fh;

	char *send_buf = new char[64*num_ranks-1];
	char *re_buf = new char[64];

	int nr_of_reads;
	if(rank == MASTER){
		MPI_File_open( MPI_COMM_SELF , FILE, MPI_MODE_RDONLY , MPI_INFO_NULL , &fh);
		MPI_Offset file_size;
		MPI_File_get_size(fh, &file_size);
		//cout<<"file_size"<<file_size<<endl;
		nr_of_reads = (double)file_size/(double)(64*(num_ranks-1));
		
	}
	MPI_Bcast(&nr_of_reads, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	
	//sendcount = 0 fore master 64 fore the rest
	int *sendCount = new int[num_ranks];
	fill(sendCount,sendCount+num_ranks,64);
	sendCount[0] = 0;
	//index zero and one are 0 maseter dont get any data
	int *displ = new int[num_ranks]{0};
	for(int k = 1; k<num_ranks;k++){
		displ[k] = (k-1)*64; 
	}
	
	//master recives 0 data
	int recvcount = (rank==MASTER) ? 0:64;
	cout<<"nr of reads "<<nr_of_reads<<endl;
	vector<map<string, pair<string,int>>> buckets(num_ranks);
	for(int i = 0; i<nr_of_reads;i++){

		if(rank == MASTER){
			MPI_File_read(fh,send_buf, 64*(num_ranks-1),MPI_BYTE, MPI_STATUS_IGNORE);
			
		}
		
		MPI_Scatterv(send_buf,sendCount,displ,MPI_BYTE,re_buf,recvcount,MPI_BYTE,MASTER,MPI_COMM_WORLD);
		if(rank!=MASTER){
			//do slave work
			long offset = 0;
			int bucket_index;
			string key;
			
			while(offset<64){
				pair<string,int> p = func_map(re_buf,offset);
				/*
				if(p.first.compare(0,4,"test")==0){
					key = p.first;
					//cout<<"test found bucket = "<<calculateDestRank(p.first.data(),p.first.length(),num_ranks)<<endl;
				}
				
				*/
				cout<<"key="<<p.first<<endl;
				if(p.first == "fail" && p.second == 0){
					break;
				}

				bucket_index = calculateDestRank(p.first.data(),p.first.length(),num_ranks);
				if(buckets[bucket_index].count(p.first)==0){
					//add
					buckets[bucket_index][p.first] = p;
				}
				else{
					//increas value
					buckets[bucket_index][p.first].second++;
				}


			}
			
		}
		//Mpi_Alltoall()
		


	}

	
	MPI_Finalize();
	return 0;
}