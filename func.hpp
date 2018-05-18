#ifndef FUNC_H
#define FUNC_H

#include <utility>
#include <ctype.h>
#include <sstream>

std::pair<std::string,int> func_map(char *buf,long &offset);

void reduce(std::pair<std::string, int> &p1, std::pair<std::string, int> p2);
#endif

