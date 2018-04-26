#include "Memory/LRUCache.hpp"
#include <iostream>
#include "Memory/TraceAllocator.hpp"
#include <vector>

int main(void) 
{
	KLib::LRUCache<int, int> cache;

	for (int cnt = 0; cnt < 100; ++cnt) {
		cache.put(cnt, cnt);
	}

	std::cout << "LRUCache allocated size : " << cache.allocatedSize() << std::endl;
	std::cout << "LRUCache max alloc size : " << cache.maxAllocSize() << std::endl;

	std::vector<int, KLib::TraceAllocator<int> > vec;
	for (int cnt = 0; cnt < 100; ++cnt) {
		vec.push_back(cnt);
	}
}
