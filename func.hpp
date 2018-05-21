#ifndef FUNC_H
#define FUNC_H

#include <utility>
#include <ctype.h>
#include <sstream>
#define KEY_MAX_SIZE 30
struct Key_value
{
	long count;
	char key[KEY_MAX_SIZE];
	 
};
Key_value func_map(char *buf,long &offset);

void reduce(Key_value &p1, Key_value p2);
#endif

