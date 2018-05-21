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
	
	const int nitems=2;
	int blocklengths[2] = {1,30};
    MPI_Datatype types[2] = {MPI_LONG, MPI_CHAR};
    MPI_Datatype mpi_key_value_type;
    MPI_Aint     offsets[2];

    offsets[0] = 0;
    offsets[1] = sizeof(long);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_key_value_type);
    MPI_Type_commit(&mpi_key_value_type);
	
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
	string key_33;
	//master recives 0 data
	int recvcount = (rank==MASTER) ? 0:64;
	cout<<"nr of reads "<<nr_of_reads<<endl;
	vector<map<string, Key_value>> buckets(num_ranks);
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

				Key_value p = func_map(re_buf,offset);
				/*
				if(p.key.compare(0,3,"abc")==0){
					key_33 = p.key;
					//cout<<"test found bucket = "<<calculateDestRank(p.key.data(),p.key.length(),num_ranks)<<endl;
				}
				*/
				string key_test = p.key;
				//cout<<"key="<<p.key<<endl;
				if(p.count == 0){
					break;
				}

				bucket_index = calculateDestRank(p.key,KEY_MAX_SIZE,num_ranks);
				cout<<"antal ggr ="<<buckets[bucket_index].count(key_test)<<" key="<<p.key<<endl;
				
				//cout<<endl;
				if(buckets[bucket_index].count(key_test)==0){
					//add
					cout<<"added "<<p.key<<" to map"<<endl;
					buckets[bucket_index][key_test] = p;
				}
				else{
					//increas value
					
					buckets[bucket_index][key_test].count++;
					cout<<"increas value to "<<buckets[bucket_index][p.key].count<<endl;
				}


			}
			
		}


		//Mpi_Alltoallv()
		//send = vektor.data
		/*
		int MPI_Alltoallv(const void *sendbuf, const int *sendcounts,
                  const int *sdispls, MPI_Datatype sendtype, void *recvbuf,
                  const int *recvcounts, const int *rdispls, MPI_Datatype recvtype,
                  MPI_Comm comm)
        */
		


	}
	
	int nr_in_bucket[num_ranks];
	//cout<<"nr of rankes"<<num_ranks<<endl;
	for(int k = 0; k<num_ranks;k++){
		nr_in_bucket[k] = buckets[k].size();
		cout<<"my rank"<<rank<<endl;
		cout<<"antal i buket "<<k<<"="<<nr_in_bucket[k]<<endl;
	}
	
	MPI_Finalize();
	return 0;
}