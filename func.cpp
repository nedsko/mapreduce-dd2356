#include <func.hpp>

// ADD -c FLAG TO COMPILE WITH OUT MAIN
//TODO fix size for char buf
std::pair<std::string,int> func_map(char *buf,long &offset){
	std::stringstream ss;
	std::pair <std::string,int> p;
	int char_count = 0;
	int key_max_size = 30;
	while(true){
		if(isdigit(buf[offset])){

			while(isdigit(buf[offset])){
				ss<<buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<key_max_size){
				char_count++;
				ss<<'\0';
			}
			std::string str = ss.str();
			p = std::make_pair (str,1);
			return p;

		}
		else if(isalpha(buf[offset])){
			while(isalpha(buf[offset])){
				ss<<buf[offset];
				char_count++;
				offset++;
			}
			while(char_count<key_max_size){
				char_count++;
				ss<<'\0';
			}
			std::string str = ss.str();
			p = std::make_pair (str,1);
			return p;
		}
		offset++;

		if(offset>64){
			p = std::make_pair ("fail",0);
			return p;
		}
	}
	

} 