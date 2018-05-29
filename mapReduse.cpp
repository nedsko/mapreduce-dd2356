#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "func.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <sys/time.h>

#define FILE "/cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_80GB.txt" // Input file
#define MASTER 0
#define NULL_STRING "fail\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define RESULT_FILE "mapreduce_results.txt"
#define PERFORMANCE_FILE "mapreduce_performance.txt"

// Function for measuring time.
double mysecond();

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
    int rank_int, num_ranks_int;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_int);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks_int);

    long rank = (long)rank_int;
    long num_ranks = (long)num_ranks_int;

    MPI_File fh;
    // Declare MPI derived data type equal to one Key_value struct
    const int nitems=2;
    int blocklengths[2] = {1,KEY_MAX_SIZE};
    MPI_Datatype types[2] = {MPI_LONG, MPI_CHAR};
    MPI_Datatype mpi_key_value_type;
    MPI_Aint offsets[2];
    offsets[0] = 0;
    offsets[1] = sizeof(long);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_key_value_type);
    MPI_Type_commit(&mpi_key_value_type);
    // Create MPI_Datatype used for individual IO.
    MPI_Aint length = READ_SIZE * sizeof(char);
    MPI_Aint extent = num_ranks * length;
    MPI_Offset disp = rank * length;
    MPI_Datatype contig, filetype;

    MPI_Type_contiguous(READ_SIZE, MPI_BYTE, &contig);
    MPI_Type_create_resized(contig, 0, extent, &filetype);
    MPI_Type_commit(&filetype);

    double init_start_time;
    /* START OF MAP PHASE */
    // Only master keeps track of time.
    if (rank == MASTER) {
        init_start_time = mysecond();
    }
    //cout<<"Process "<<rank<<": Map Phase started!"<<endl;

    // Buffers for reading data from input file
    char *re_file_data = new char[READ_SIZE];

    long long nr_of_reads; // Number of times the file will be read
    MPI_Offset file_size;
    // Set views used for individual IO.
    MPI_File_open(MPI_COMM_WORLD , FILE, MPI_MODE_RDONLY , MPI_INFO_NULL , &fh);
    MPI_File_set_view(fh, disp, MPI_BYTE, filetype, "native", MPI_INFO_NULL);
    MPI_File_get_size(fh, &file_size);

    nr_of_reads = file_size/(READ_SIZE*(num_ranks));
    // Handle case where size of file is not aligned with number_of_reads and num_ranks
    long miss = file_size - nr_of_reads*READ_SIZE*num_ranks; // Number of bytes left trailing at EOF.
    long extra_reads = miss/READ_SIZE;
    // Recruit one process per trailing chunks of size READ_SIZE.
    // Must be in order of ranks. See report for more details.
    if(rank < extra_reads){
        nr_of_reads +=1;
    }
    // Vector used to separate <key,value> pairs into buckets.
    // Use individual IO so every process reads from the input file
    vector<map<string, Key_value> > buckets(num_ranks);
    for(long long i = 0; i<nr_of_reads;i++){
        MPI_File_read(fh, re_file_data, READ_SIZE, MPI_BYTE, MPI_STATUS_IGNORE);
        // Repeatedly call Map() on read buffer until all data has been processed
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
            // Calc. buckets hash for found word.
            bucket_index = calculateDestRank(p.key,KEY_MAX_SIZE,num_ranks);
            if(buckets[bucket_index].count(key_test)==0){
                buckets[bucket_index][key_test] = p;
            } else {
                buckets[bucket_index][key_test].count++;
            }

        }
    }
    /* MAP PHASE DONE! */
    //cout<<"Process "<<rank<<": Map Phase done!"<<endl;

    /* REDISTRIBUTION OF KEY VALUES */
    //cout<<"Process "<<rank<<": Redistribution Phase started!"<<endl;

    // Calculate the size of the biggest bucket. Used for padding on Alltoall later
    long elements_in_bucket[num_ranks];
    long total_bucket_size = 0;
    long local_max_bucket_size = 0;
    long global_max_bucket_size = 0;
    for(int k = 0; k<num_ranks;k++){
        elements_in_bucket[k] = buckets[k].size();
        if(local_max_bucket_size<elements_in_bucket[k])
        local_max_bucket_size = elements_in_bucket[k];
        total_bucket_size += buckets[k].size();
    }
    // Use Allreduce to calculate and share global_max_bucket_size
    MPI_Allreduce(&local_max_bucket_size, &global_max_bucket_size, 1,MPI_LONG, MPI_MAX, MPI_COMM_WORLD);

    // Create, pad and redistribute data to all other processes
    vector<Key_value> send_vector;
    vector<Key_value> recv_vector(global_max_bucket_size*num_ranks);
    // Fill and pad send_vector
    Key_value *temp_key_value = new Key_value;
    temp_key_value->count = 0;
    strncpy(temp_key_value->key, NULL_STRING, KEY_MAX_SIZE);
    for(int b = 0; b<buckets.size();b++){
        int curent_size = 0;
        for(map<string, Key_value>::iterator it = buckets[b].begin(); it!=buckets[b].end();++it){
            send_vector.push_back(it->second);
            curent_size++;
        }
        for(long i = curent_size;curent_size<global_max_bucket_size;curent_size++){
            send_vector.push_back(*temp_key_value);
        }
    }
    // Redistribute data using Alltoall
    MPI_Alltoall(send_vector.data(),global_max_bucket_size,mpi_key_value_type,recv_vector.data(),global_max_bucket_size,mpi_key_value_type,MPI_COMM_WORLD);

    /* REDISTRIBUTION OF KEY VALUES DONE! */
    //cout<<"Process "<<rank<<": Redistribution Phase done!"<<endl;

    /* START OF REDUCE PHASE */
    //cout<<"Process "<<rank<<": Reduce Phase started!"<<endl;

    // Build map object from recv_vector and call reduce() on the data
    map<string, Key_value> agg_key_value_map; // agg = aggregated
    for(vector<Key_value>::iterator it = recv_vector.begin(); it != recv_vector.end();++it){
        string key_test = it->key;
        // Padding, don't include
        if(it->count == 0)
        continue;
        // if key is in map then call reduce, otherwise add to map
        if(agg_key_value_map.count(key_test)>0){
            reduce(agg_key_value_map[key_test],*it);
        } else {
            agg_key_value_map[key_test] = *it;
        }
    }
    /* REDUCE PHASE DONE! */
    //cout<<"Process "<<rank<<": Reduce Phase done!"<<endl;

    /* GATHER RESULTS PHASE */
    //cout<<"Process "<<rank<<": Gather Phase started!"<<endl;

    // Create and fill send vector with aggregated values
    vector<Key_value> send_vector_agg;
    for(map<string, Key_value>::iterator  it = agg_key_value_map.begin();it != agg_key_value_map.end();++it){
        send_vector_agg.push_back(it->second);
    }
    // Calculate size of biggest send_vector to use for padding
    long local_send_vector_agg_size = send_vector_agg.size();
    long max_send_vector_agg_size;
    MPI_Allreduce(&local_send_vector_agg_size, &max_send_vector_agg_size, 1,MPI_LONG, MPI_MAX, MPI_COMM_WORLD);
    // Add padding
    for(long i = local_send_vector_agg_size;i<max_send_vector_agg_size;i++){
        send_vector_agg.push_back(*temp_key_value);
    }
    vector<Key_value> recv_vector_agg(max_send_vector_agg_size*num_ranks);
    // Gather results to MASTER
    MPI_Gather(send_vector_agg.data(), max_send_vector_agg_size, mpi_key_value_type,recv_vector_agg.data(), max_send_vector_agg_size, mpi_key_value_type,MASTER, MPI_COMM_WORLD);

    /* GATHER PHASE DONE! */
    //cout<<"Process "<<rank<<": Gather Phase done!"<<endl;

    // Print results and performance measurment to files.
    if(rank == MASTER) {
        double total_runtime = mysecond() - init_start_time;
        char line_buffer[100+KEY_MAX_SIZE];
        int line_length;
        // Print performance file
        MPI_File performance_file;
        MPI_File_open(MPI_COMM_SELF, PERFORMANCE_FILE, MPI_MODE_CREATE | MPI_MODE_APPEND | MPI_MODE_WRONLY, MPI_INFO_NULL, &performance_file);
        line_length = sprintf(line_buffer, "File: %s, Size: %lld bytes, #Processes: %ld, Runtime: %11.8f seconds\n", FILE, file_size, num_ranks, total_runtime);
        if (line_length > 0) {
            MPI_File_write(performance_file, line_buffer, line_length, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        MPI_File_close(&performance_file);

        // Print results to result file
        MPI_File result_file;
        long unique_words = 0;
        MPI_File_open(MPI_COMM_SELF, RESULT_FILE, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &result_file);
        for (vector<Key_value>::iterator it = recv_vector_agg.begin();it != recv_vector_agg.end();++it) {
            if(it->count!=0){
                line_length = sprintf(line_buffer, "Word: %s, count: %ld\n", it->key, it->count);
                //cout<<line_buffer<<endl;
                if (line_length > 0) {
                    //MPI_File_write(result_file, line_buffer, line_length, MPI_CHAR, MPI_STATUS_IGNORE);
                    unique_words++;
                }
            }
        }
        line_length = sprintf(line_buffer, "Number of unique words: %ld", unique_words);
        if (line_length > 0) {
            MPI_File_write(result_file, line_buffer, line_length, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        cout<<"num uniq words "<<unique_words<<endl;
        MPI_File_close(&result_file);
    }
    // Free allocated memory
    MPI_Type_free(&mpi_key_value_type);
    MPI_Type_free(&filetype);
    delete[] re_file_data;
    delete temp_key_value;
    MPI_Finalize();
    return 0;
}

// Function returning current time with milliseconds.
// Taken from assignment 1 in the course.
double mysecond() {
    struct timeval tp;
    struct timezone tzp;
    int i;
    i = gettimeofday(&tp,&tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}
