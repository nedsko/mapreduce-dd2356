
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

int main(int argc, char *argv[]){

	
	string s = "1234¥45hej bla@test.se";
	char *temp = new char[s.length()+1];
	strcpy(temp,s.c_str());
	long of = 0;
	std:pair<string,int> p = map(temp,of);
	cout<<p.first<<endl;

	return 0;

}
