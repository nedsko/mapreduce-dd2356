#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "func.hpp"
#include <vector>
#include <map>
#include <cstdint>

#define FILE "wikipedia_test_small.txt" // Input file
#define MASTER 0
#define NULL_STRING "fail\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

//wikipedia_test_small.txt number_test.txt
using namespace std;

// Hash function used to decide bucket for given word
#define SEED_LENGTH 65
const char key_seed[SEED_LENGTH] = "b4967483cf3fa84a3a233208c129471ebc49bdd3176c8fb7a2c50720eb349461";
const unsigned short *key_seed_num = (unsigned short*)key_seed;

int calculateDestRank(const char *word, int length, int num_ranks) {
    uint64_t hash = 0;
    for (uint64_t i = 0; i < length; i++) {
        uint64_t num_char = (uint64_t)word[i];
        uint64_t seed     = (uint64_t)key_seed_num[(i % SEED_LENGTH)];

        hash += num_char * seed * (i + 1);
    }
    return (int)(hash % (uint64_t)num_ranks);
}

// Word counting program
int main(int argc, char *argv[]){
	int rank, num_ranks;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

	MPI_File fh;
  // Declare MPI derived data type equal to one Key_value struct
	const int nitems=2;
	int blocklengths[2] = {1,30};
  MPI_Datatype types[2] = {MPI_LONG, MPI_CHAR};
  MPI_Datatype mpi_key_value_type;
  MPI_Aint offsets[2];
  offsets[0] = 0;
  offsets[1] = sizeof(long);
  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_key_value_type);
  MPI_Type_commit(&mpi_key_value_type);

  /* START OF MAP PHASE */
  // Buffers for sending/receiving data from input file
	char *send_read_file_data = new char[READ_SIZE*num_ranks-1];
	char *re_file_data = new char[READ_SIZE];

	int nr_of_reads; // Number of times the file will be read by MASTER
	if(rank == MASTER){
		MPI_File_open(MPI_COMM_SELF , FILE, MPI_MODE_RDONLY , MPI_INFO_NULL , &fh);
		MPI_Offset file_size;
		MPI_File_get_size(fh, &file_size);
		nr_of_reads = (double)file_size/(double)(READ_SIZE*(num_ranks-1));
	}
  // Broadcast nr_of_reads to all processes
	MPI_Bcast(&nr_of_reads, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  // Prepare send to slave processes
	int *sendCount = new int[num_ranks];
	fill(sendCount,sendCount+num_ranks,READ_SIZE);
  // Master does not receive any data from file so sendCount[0] = 0 and displ[0:1] = 0
	sendCount[0] = 0;
	int *displ = new int[num_ranks]{0};
	for(int k = 1; k<num_ranks;k++){
		displ[k] = (k-1)*READ_SIZE;
	}
	int recvcount = (rank==MASTER) ? 0:READ_SIZE;
  // Vector used to separate <key,value> pairs into buckets.
	vector<map<string, Key_value>> buckets(num_ranks);

	for(int i = 0; i<nr_of_reads;i++){
    // Only master reads from file
		if(rank == MASTER){
			MPI_File_read(fh,send_read_file_data, READ_SIZE*(num_ranks-1),MPI_BYTE, MPI_STATUS_IGNORE);
		}
    // Scatter read data to slave processes
		MPI_Scatterv(send_read_file_data,sendCount,displ,MPI_BYTE,re_file_data,recvcount,MPI_BYTE,MASTER,MPI_COMM_WORLD);
    // Repeatedly call Map() on received buffer until all data has been processed
		if(rank != MASTER){
			long offset = 0;
			int bucket_index;
			string key;

			while(offset<READ_SIZE){
				Key_value p = func_map(re_file_data,offset);
				string key_test = p.key;
        // Check that we did not go over allowed offset
				if(p.count == 0){
					break;
				}
				bucket_index = calculateDestRank(p.key,KEY_MAX_SIZE,num_ranks);
				if(buckets[bucket_index].count(key_test)==0){
					buckets[bucket_index][key_test] = p;
				} else {
					buckets[bucket_index][key_test].count++;
				}
			}
		}
	}
  /* MAP PHASE DONE! */

  /* REDISTRIBUTION OF KEY VALUES */
  // Calculate the size of the biggest bucket. Used for padding later
	int elements_in_bucket[num_ranks];
	int total_bucket_size = 0;
	int local_max_bucket_size = 0;
	int global_max_bucket_size = 0;
	for(int k = 0; k<num_ranks;k++){
		elements_in_bucket[k] = buckets[k].size();
		if(local_max_bucket_size<elements_in_bucket[k])
			local_max_bucket_size = elements_in_bucket[k];
		total_bucket_size += buckets[k].size();
	}
  // Use Allreduce to calculate and share global_max_bucket_size
	MPI_Allreduce(&local_max_bucket_size, &global_max_bucket_size, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  // Create, pad and redistribute data to all other processes
	vector<Key_value> *send_vector = new vector<Key_value>;
	vector<Key_value> *recv_vector = new vector<Key_value>(global_max_bucket_size*num_ranks);
  // Fill and pad send_vector
	for(int b = 0; b<num_ranks;b++){
		int curent_size = 0;
		for(auto it = buckets[b].begin(); it!=buckets[b].end();++it){
			send_vector->push_back(it->second);
			curent_size++;
		}
		for(int i = curent_size;curent_size<global_max_bucket_size;curent_size++){
      // TODO: Move temp_key_value outside to prevent excessive string copying
			Key_value *temp_key_value = new Key_value;
			temp_key_value->count = 0;
			strncpy(temp_key_value->key, NULL_STRING, 30);
			send_vector->push_back(*temp_key_value);
		}
	}
  // Redistribute data using Alltoall
	MPI_Alltoall(send_vector->data(),global_max_bucket_size,mpi_key_value_type,recv_vector->data(),global_max_bucket_size,mpi_key_value_type,MPI_COMM_WORLD);

  /* REDISTRIBUTION OF KEY VALUES DONE! */

  /* START OF REDUCE PHASE */
  // Build map object from recv_vector and call reduce() on the data
	map<string, Key_value> agg_key_value_map; // agg = aggregated
	for(auto it = recv_vector->begin(); it != recv_vector->end();++it){
		string key_test = it->key;
    // Padding, don't include
		if(it->count == 0)
			continue;
    // key is in map then call reduce, otherwise add to map
		if(agg_key_value_map.count(key_test)>0){
			reduce(agg_key_value_map[key_test],*it);
		} else {
			agg_key_value_map[key_test] = *it;
		}
	}
  /* REDUCE PHASE DONE! */
  /* GATHER RESULTS PHASE */
  // Create and fill send vector with aggregated values
	vector<Key_value> *send_vector_agg = new vector<Key_value>;
	for(auto it = agg_key_value_map.begin();it != agg_key_value_map.end();++it){
		send_vector_agg->push_back(it->second);
	}
  // Calculate size of biggest send_vector to use for padding
	int local_send_vector_agg_size = send_vector_agg->size();
	int max_send_vector_agg_size;
	MPI_Allreduce(&local_send_vector_agg_size, &max_send_vector_agg_size, 1,MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  // Add padding
	for(int i = local_send_vector_agg_size;i<max_send_vector_agg_size;i++){
    //TODO move empty key_value out of loop
		Key_value *temp_key_value = new Key_value;
		temp_key_value->count = 0;
		strncpy(temp_key_value->key, NULL_STRING, 30);
		send_vector_agg->push_back(*temp_key_value);
	}
	vector<Key_value> *recv_vector_agg = new vector<Key_value>(max_send_vector_agg_size*num_ranks);
  // Gather results to MASTER
	MPI_Gather(send_vector_agg->data(), max_send_vector_agg_size, mpi_key_value_type,recv_vector_agg->data(), max_send_vector_agg_size, mpi_key_value_type,MASTER, MPI_COMM_WORLD);

  /* GATHER PHASE DONE! */
	// TODO FREE KEY VALUES inside objects when they aren't needed anymore
  // TODO: Add measure time etc. and print measurements to file
  // TODO: Print results to file

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
  // Free allocated memory
  MPI_Type_free(&mpi_key_value_type);
  delete[] send_read_file_data;
  delete[] re_file_data;
  delete[] sendCount;
  delete[] displ;
  delete send_vector;
  delete recv_vector;
  delete send_vector_agg;
  delete recv_vector_agg;

	MPI_Finalize();
	return 0;
}
