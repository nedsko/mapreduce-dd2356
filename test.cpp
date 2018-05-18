
#include <iostream>
#include <string>
#include <func.hpp>
#include <assert.h>
#include <utility>
#include <string.h>
#include <cstring>
#define FILE "wikipedia_test_small.txt"
#define MASTER 0
using namespace std;


// ADD -c FLAG TO COMPILE WITH OUT MAIN
/*
pair<string,int> map(char *buf,long &offset){
	stringstream ss;
	pair <string,int> p;
	while(true){
		
		if(isdigit(buf[offset])){
			cout<<"space is digit"<<endl;
			while(isdigit(buf[offset])){
				ss<<buf[offset];
				offset++;
			}
			string str = ss.str();
			p = make_pair (str,1);
			return p;

		}
		else if(isalpha(buf[offset])){
			cout<<"space is alpha"<<endl;
			while(isalpha(buf[offset])){

				ss<<buf[offset];
				offset++;
			}
			string str = ss.str();
			p = make_pair (str,1);
			return p;
		}
		offset++;
		cout<<"space is nothing"<<endl;
		if(offset>64){ 
			p = make_pair ("fail",0);
			cout<<"faile ret"<<endl;
			return p;
		}
	}
	

} 
*/
int main(int argc, char *argv[]){

	
	string s = "test@1234 hej$ bla@test.se                                                                      ";//"test 1234 hej bla@test.se";
	char *temp = new char[s.length()+1];
	strcpy(temp,s.c_str());
	long of = 0;
	for(int i = 0; i<10;i++){
		pair<string,int> p = map(temp,of);
		cout<<p.first<<" value"<<p.second<<endl;	
	}
	
	return 0;

}
