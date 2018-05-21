#include "func.hpp"


// ADD -c FLAG TO COMPILE WITH OUT MAIN
//TODO fix size for char buf
Key_value func_map(char *buf,long &offset){
	std::stringstream ss;
	struct Key_value *p = new Key_value;
	int char_count = 0;
	while(true){
		if(isdigit(buf[offset])){

			while(isdigit(buf[offset])){
				p->key[char_count]=buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<KEY_MAX_SIZE){
				p->key[char_count]='\0';
				char_count++;
			}
			p->count=1;
			return *p;

		}
		else if(isalpha(buf[offset])){
			while(isalpha(buf[offset])){
				p->key[char_count]=buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<KEY_MAX_SIZE){
				p->key[char_count]='\0';
				char_count++;
			}
			p->count=1;
			return *p;
		}
		offset++;

		if(offset>64){
			while(char_count<KEY_MAX_SIZE){
				p->key[char_count]='\0';
				char_count++;
			}
			p->count=0;
			return *p;
		}
	}


}

void reduce(Key_value &p1, Key_value p2){
	p1.count += p2.count;
}
