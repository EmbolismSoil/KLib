#include "Memory/LRUCache.hpp"
#include <iostream>

int main(void) 
{
	KLib::LRUCache<int, int> cache;

	for (int cnt = 0; cnt < 100; ++cnt) {
		cache.put(cnt, cnt);
	}

	std::cout << cache.allocatedSize() << std::endl;
}