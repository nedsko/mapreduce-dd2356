#include <func.hpp>

// ADD -c FLAG TO COMPILE WITH OUT MAIN
//TODO fix size for char buf
std::pair<std::string,int> map(char *buf,long &offset){
	std::stringstream ss;
	std::pair <std::string,int> p;
	while(true){
		if(isdigit(buf[offset])){
			while(isdigit(buf[offset])){
				ss<<buf[offset];
				offset++;
			}
			std::string str = ss.str();
			p = std::make_pair (str,1);
			return p;

		}
		else if(isalpha(buf[offset])){
			while(isalpha(buf[offset])){
				ss<<buf[offset];
				offset++;
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