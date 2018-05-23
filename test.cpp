
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

int main(int argc, char *argv[]){

	string s = "test@1234 hej$ bla@test.se                                                                      ";//"test 1234 hej bla@test.se";
	char *temp = new char[s.length()+1];
	strcpy(temp,s.c_str());
	long of = 0;
	for(int i = 0; i<10;i++){
		struct Key_value p = func_map(temp,of);

		cout<<p.key<<" value"<<*p.count<<endl;
	}

	return 0;

}
