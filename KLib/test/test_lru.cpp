#include "Memory/LRUCache.hpp"
#include <iostream>

int main(void) 
{
	KLib::LRUCache<int, int, 100> cache;

	for (int cnt = 0; cnt < 100; ++cnt) {
		cache.put(cnt, cnt);
	}

	std::cout << "allocated size : " << cache.allocatedSize() << std::endl;
	std::cout << "max alloc size : " << cache.maxAllocSize() << std::endl;
}