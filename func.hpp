// Header file for MapReduce functons
#ifndef FUNC_H
#define FUNC_H

#include <utility>
#include <ctype.h>
#include <sstream>
#define KEY_MAX_SIZE 30 // Maximum length of a key(word)
#define READ_SIZE 256  // Size of chunks read from file and sent to one process
// Struct used to hold <key,value> pairs
struct Key_value
{
	long count;
	char key[KEY_MAX_SIZE];

};
// Map() in MapReduce programming model
Key_value func_map(char *buf,long &offset);

// Reduce() in MapReduce programming model
void reduce(Key_value &p1, Key_value p2);
#endif
