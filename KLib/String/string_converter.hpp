#ifndef __STRINGCONVERTER_HPP__
#define __STRINGCONVERTER_HPP__
#include <string>
#include <stdlib.h>
#include <sstream>
#include <iostream>

template<class T>
T strto(std::string const& s) 
{
	std::istringstream ss(s);
	T num;
	ss >> num;
	return num;
}

template<class T>
std::string tostr(T const& t) 
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}
#endif // !__STRINGCONVERTER_HPP__
