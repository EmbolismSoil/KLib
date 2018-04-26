#include "Memory/LRUCache.hpp"
#include <iostream>
#include "Memory/TraceAllocator.hpp"
#include <vector>
#include <string>
#include <sstream>

int main(void) 
{
        typedef std::basic_string<char, std::char_traits<char>, KLib::TraceAllocator<char> > cache_string;
	KLib::LRUCache<int, cache_string, 100> cache;
        
	for (int cnt = 0; cnt < 100; ++cnt) {
                cache_string s(cnt, 'c');
		cache.put(cnt, s);
	}

	std::cout << "LRUCache allocated size : " << cache.allocatedSize() << std::endl;
	std::cout << "LRUCache max alloc size : " << cache.maxAllocSize() << std::endl;

	std::vector<int, KLib::TraceAllocator<int, 100> > vec;
	for (int cnt = 0; cnt < 100; ++cnt) {
		vec.push_back(cnt);
	}

        std::cout << "vector allocated size : " << vec.get_allocator().allocatedSize() << std::endl;
}
