#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "func.hpp"
#include <vector>
#include <map>
#include <cstdint>
#define FILE "wikipedia_test_large.txt"
#define MASTER 0

#define NULL_STRING "fail\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"


//wikipedia_test_small.txt number_test.txt
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
	
	char *send_buf = new char[READ_SIZE*num_ranks-1];
	char *re_buf = new char[READ_SIZE];

	int nr_of_reads;
	if(rank == MASTER){
		MPI_File_open( MPI_COMM_SELF , FILE, MPI_MODE_RDONLY , MPI_INFO_NULL , &fh);
		MPI_Offset file_size;
		MPI_File_get_size(fh, &file_size);
		nr_of_reads = (double)file_size/(double)(READ_SIZE*(num_ranks-1));
		
	}
	MPI_Bcast(&nr_of_reads, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	
	//sendcount = 0 fore master READ_SIZE fore the rest
	int *sendCount = new int[num_ranks];
	fill(sendCount,sendCount+num_ranks,READ_SIZE);
	sendCount[0] = 0;
	//index zero and one are 0 maseter dont get any data
	int *displ = new int[num_ranks]{0};
	for(int k = 1; k<num_ranks;k++){
		displ[k] = (k-1)*READ_SIZE; 
	}
	string key_33;
	//master recives 0 data
	int recvcount = (rank==MASTER) ? 0:READ_SIZE;
	vector<map<string, Key_value>> buckets(num_ranks);
	//map loop
	for(int i = 0; i<nr_of_reads;i++){

		if(rank == MASTER){
			MPI_File_read(fh,send_buf, READ_SIZE*(num_ranks-1),MPI_BYTE, MPI_STATUS_IGNORE);
			
		}
		
		MPI_Scatterv(send_buf,sendCount,displ,MPI_BYTE,re_buf,recvcount,MPI_BYTE,MASTER,MPI_COMM_WORLD);
		if(rank!=MASTER){
			//do slave work
			long offset = 0;
			int bucket_index;
			string key;
			
			while(offset<READ_SIZE){

				Key_value p = func_map(re_buf,offset);
				
				string key_test = p.key;
				if(p.count == 0){
					break;
				}
				bucket_index = calculateDestRank(p.key,KEY_MAX_SIZE,num_ranks);
				
				if(buckets[bucket_index].count(key_test)==0){
					//add
					buckets[bucket_index][key_test] = p;
				}
				else{
					//increas value
					
					buckets[bucket_index][key_test].count++;
				}
			}	
		}
	}

	int nr_in_bucket[num_ranks];
	int total_bucket_size = 0;
	int max = 0;
	int re_global_max = 0;
	int send_displ[num_ranks];
	for(int k = 0; k<num_ranks;k++){
		nr_in_bucket[k] = buckets[k].size();
		if(max<nr_in_bucket[k])
			max = nr_in_bucket[k];
		total_bucket_size += buckets[k].size();
	}
	
	MPI_Allreduce(&max, &re_global_max, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	vector<Key_value> *send_vector = new vector<Key_value>;
	vector<Key_value> *recv_vector = new vector<Key_value>(re_global_max*num_ranks);
	
	for(int b = 0; b<num_ranks;b++){
		int curent_size = 0;
		for(auto it = buckets[b].begin(); it!=buckets[b].end();++it){
			send_vector->push_back(it->second);
			curent_size++;
		}
		for(int i = curent_size;curent_size<re_global_max;curent_size++){
			Key_value *temp_key_value = new Key_value;
			temp_key_value->count = 0;
			strncpy(temp_key_value->key, NULL_STRING, 30);
			
			send_vector->push_back(*temp_key_value);
		}	
	}
	

	MPI_Alltoall(send_vector->data(),re_global_max,mpi_key_value_type,recv_vector->data(),re_global_max,mpi_key_value_type,MPI_COMM_WORLD);

	cout<<"recev"<<endl;
		
	map<string, Key_value> agg_key_value_map;
	for(auto it = recv_vector->begin(); it != recv_vector->end();++it){
		string key_test = it->key;
		if(it->count == 0)
			continue;
		if(agg_key_value_map.count(key_test)>0){
			reduce(agg_key_value_map[key_test],*it);
		}
		else{
			agg_key_value_map[key_test] = *it;
		}
		
	}
	vector<Key_value> *send_vector_agg = new vector<Key_value>;
	for(auto it = agg_key_value_map.begin();it != agg_key_value_map.end();++it){
		send_vector_agg->push_back(it->second);
	}
	int re_max_map_size;
	int vector_send_size = send_vector_agg->size();
	
	
	int my_send_vector_agg_size = send_vector_agg->size();
	int max_send_vector_agg_size; 
	MPI_Allreduce(&my_send_vector_agg_size, &max_send_vector_agg_size, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	//TODO move empty key_value out of loop
	for(int i = my_send_vector_agg_size;i<max_send_vector_agg_size;i++){
		Key_value *temp_key_value = new Key_value;
		temp_key_value->count = 0;
		strncpy(temp_key_value->key, NULL_STRING, 30);
		send_vector_agg->push_back(*temp_key_value);
	}

	vector<Key_value> *recv_vector_agg = new vector<Key_value>(max_send_vector_agg_size*num_ranks);
	MPI_Gather(send_vector_agg->data(), max_send_vector_agg_size, mpi_key_value_type,recv_vector_agg->data(), max_send_vector_agg_size, mpi_key_value_type,MASTER, MPI_COMM_WORLD);
	
	
	
	
	//TODO mpi type free and free alloced memory
	
	if(rank == MASTER){
		int c = 0;
		//cout<<"gatherd size "<<recv_vector_agg->size()<<endl;
		cout<<endl<<endl;
		for(auto it = recv_vector_agg->begin();it != recv_vector_agg->end();++it){
			if(it->count!=0){
				//cout<<"key = "<<it->key<<" value "<<it->count<<endl;
				c++;
			}
			//cout<<"loop start"<<endl;
			//
			//cout<<"loop end"<<endl;
		}
		cout<<"nr of returned values "<<c<<endl;
	}
	
	//MPI_Finalize();
	return 0;
}