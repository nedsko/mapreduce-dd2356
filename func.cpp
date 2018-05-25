// Implementation file for MapReduce functions
#include "func.hpp"

/* Map() in MapReduce programming model.
 * Returns the first word(as defined in project instructions) found
 * in the buffer as a Key_value struct with count 1. Pads the word with
 * '\0' until it reaches length KEY_MAX_SIZE and moves the offset forward.
 * Words longer than KEY_MAX_SIZE will be split. If the offset goes over READ_SIZE
 * a null string with count = 0 is returned.
 */
Key_value func_map(char *buf,long &offset){
	std::stringstream ss;
	struct Key_value p;
	int char_count = 0;
	while(true){
		// First char is a digit so only digits should follow
		if(isdigit(buf[offset])){
			while(isdigit(buf[offset])&&char_count<KEY_MAX_SIZE){
				p.key[char_count]=buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<KEY_MAX_SIZE){
				p.key[char_count]='\0';
				char_count++;
			}
			p.count=1;
			return p;

		}
		// First char is alphabetic so only alphabetic chars should follow
		else if(isalpha(buf[offset])){
			while(isalpha(buf[offset])&&char_count<KEY_MAX_SIZE){
				p.key[char_count]=buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<KEY_MAX_SIZE){
				p.key[char_count]='\0';
				char_count++;
			}
			p.count=1;
			return p;
		}
		offset++; // No valid char found so move to next

		if(offset>=READ_SIZE){
			while(char_count<KEY_MAX_SIZE){
				p.key[char_count]='\0';
				char_count++;
			}
			p.count=0;
			return p;
		}
	}
}

// Reduce() function in MapReduce. Takes two Key_value structs, p1 and p2,
// and adds p2.count to p1.count.
void reduce(Key_value &p1, Key_value p2){
	p1.count += p2.count;
}
