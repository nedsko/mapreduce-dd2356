#ifndef FUNC_H
#define FUNC_H

#include <utility>
#include <ctype.h>
#include <sstream>
#define KEY_MAX_SIZE 30
struct Key_value
{
	long count;
	char *key;
	Key_value() {
		count = 0;
		key = new char[KEY_MAX_SIZE];
	};
	Key_value(long count, char* buf) {
		this->count = count;
		this->key = new char[KEY_MAX_SIZE];
		for (size_t i = 0; i < KEY_MAX_SIZE; i++) {
			*(key+i) = *(buf+i);
		}
	};
	~Key_value() {
		delete[] key;
	};
	Key_value(const Key_value &key_val2) {
		count = key_val2.count;
		key = new char[KEY_MAX_SIZE];
 	 *key = *key_val2.key;
	}
	/*
	Key_value& operator=(Key_value& a) {
	 count = a.count;
	 key = new char[KEY_MAX_SIZE];
	 *key = *a.key;
	 return *this;
	}
	*/
};
Key_value func_map(char *buf,long &offset);

void reduce(Key_value &p1, Key_value p2);
#endif
