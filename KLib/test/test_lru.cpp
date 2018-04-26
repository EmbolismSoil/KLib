#include "Memory/LRUCache.hpp"
#include <iostream>
#include "Memory/TraceAllocator.hpp"
#include <vector>
#include <string>
#include <sstream>

int main(void) 
{
	KLib::LRUCache<std::string, 100> cache;
        
	for (int cnt = 0; cnt < 100; ++cnt) {
		std::stringstream ss;
		ss << cnt;
		std::string k(ss.str());
                std::string s(cnt, 'c');
		cache.put(k, s);
	}

	std::cout << "LRUCache allocated size : " << cache.allocatedSize() << std::endl;

	KLib::LRUCache<std::vector<int>, 100> vcache;
        
	for (int cnt = 0; cnt < 100; ++cnt) {
		std::stringstream ss;
		ss << cnt;
		std::string k(ss.str());
		std::vector<int> v;
		v.push_back(cnt);
		vcache.put(k, v);
	}
	std::cout << "LRUCache allocated size : " << cache.allocatedSize() << std::endl;	
}
