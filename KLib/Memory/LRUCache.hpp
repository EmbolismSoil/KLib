#ifndef LRUCACHE_HPP
#define LRUCACHE_HPP

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <list>
#include <algorithm>
#include "time.h"
#include <numeric>
#include "TraceAllocator.hpp"
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <deque>
#include <queue>
#include <string>

#define __DECL_IS_CONTAINER(_container) \
template<class T, class Alloc> \
struct is_container<_container<T, Alloc> > \
{\
	static const bool value;\
};\
template<class T, class Alloc>\
const bool is_container<_container<T, Alloc> >::value(true)


#define __DECL_CACHE_TYPE(_container) \
template<class T, class Alloc> \
struct cache_type<_container<T, Alloc> > \
{\
	typedef _container<cache_type<T>::type, TraceAllocator<T> > type;\
}

namespace KLib
{
	typedef enum { CONTAINER = 0, NOT_CONTAINER }E_IS_CONTAINER;
	typedef std::basic_string<char, std::char_traits<char>, TraceAllocator<char> > cache_string;

	template<class T>
	struct is_container
	{
		static const bool value;
	};

	__DECL_IS_CONTAINER(std::vector);
	__DECL_IS_CONTAINER(std::list);
	__DECL_IS_CONTAINER(std::deque);
	__DECL_IS_CONTAINER(std::map);
	__DECL_IS_CONTAINER(std::set);
	__DECL_IS_CONTAINER(std::multimap);
	__DECL_IS_CONTAINER(std::multiset);
	__DECL_IS_CONTAINER(std::stack);
	__DECL_IS_CONTAINER(std::deque);
	__DECL_IS_CONTAINER(std::priority_queue);

	template<class T>
	struct cache_type 
	{
		typedef T type;
	};

	template<>
	struct cache_type<std::string> 
	{
		typedef cache_string type;
	};

	__DECL_CACHE_TYPE(std::vector);
	__DECL_CACHE_TYPE(std::list);
	__DECL_CACHE_TYPE(std::deque);
	__DECL_CACHE_TYPE(std::map);
	__DECL_CACHE_TYPE(std::set);
	__DECL_CACHE_TYPE(std::multimap);
	__DECL_CACHE_TYPE(std::multiset);
	__DECL_CACHE_TYPE(std::stack);
	__DECL_CACHE_TYPE(std::deque);
	__DECL_CACHE_TYPE(std::priority_queue);

	template<class VT>
	struct __LRUCacheNode {
		cache_string k;
		cache_type<VT>::type v;
		Node(std::string const& _k, VT const& _v) : k(_k.begin(), _k.end()), v(_v) {}
		Node(cache_string const& _k, VT const& _v) : k(_k), v(_v) {}
	};
	
	template<class __KEY_TYPE, class __VALUE_TYPE, size_t max_alloc_size=KLib::__max_size_t_limit>
	class LRUCache : boost::noncopyable {
	public:
		struct Node {
			__KEY_TYPE k;
			__VALUE_TYPE v;
			Node(__KEY_TYPE const& _k, __VALUE_TYPE const& _v) :k(_k), v(_v) {}
		};

		typedef std::list<Node, TraceAllocator<Node, max_alloc_size> > CacheList;
		typedef boost::unordered_map<__KEY_TYPE, typename CacheList::iterator, TraceAllocator<std::pair<const __KEY_TYPE, typename CacheList::iterator>, max_alloc_size > > KeyMap;

		LRUCache()
		{

		}

		__VALUE_TYPE get(__KEY_TYPE const & key)
		{
			++_reqCnt;
            size_t max = 1000;
            size_t zero = 0;

			if (_reqCnt.compare_exchange_strong(max, zero)) {
				//do sample
				if (_samples.size() >= 360) {
					_samples.pop_back();
				}

				double rate = double(_cacheHitCnt.load(boost::memory_order_acquire)) / 1000.0;
				_samples.push_front(rate);
				_cacheHitCnt.store(0, boost::memory_order_release);

				double sum = std::accumulate(_samples.begin(), _samples.end(), double(0.0));
				int hitRate = int((sum / double(_samples.size())) * 100);
				_hitRage.store(hitRate, boost::memory_order_release);
			}

			{
				boost::lock_guard<boost::mutex> guard(_cacheMtx);
				typename boost::unordered_map<__KEY_TYPE, typename CacheList::iterator>::iterator pos;

				if ((pos = _keyMap.find(key)) != _keyMap.end()) { //hit
					_cacheHitCnt.fetch_add(1, boost::memory_order_release);
					typename CacheList::iterator vit = pos->second;
					_cacheList.splice(_cacheList.begin(), _cacheList, vit);
					_keyMap[key] = _cacheList.begin();
					return vit->v;
				}
			}


			throw std::range_error("not hit");
		}

		void put(__KEY_TYPE const& k, __VALUE_TYPE const& v)
		{
			{
				boost::lock_guard<boost::mutex> guard(_cacheMtx);

				typename boost::unordered_map<__KEY_TYPE, typename CacheList::iterator>::iterator pos;
				if ((pos = _keyMap.find(k)) != _keyMap.end()) {
					typename CacheList::iterator vit = pos->second;
					vit->v = v; //update
					_cacheList.splice(_cacheList.begin(), _cacheList, vit);
					_keyMap[k] = _cacheList.begin();
					return;
				}
			}

#if 0
			{
				boost::lock_guard<boost::mutex> guard(_cacheMtx);
				if (_cacheList.size() >= _capacity) {
					Node const& endNode = _cacheList.back();
					_keyMap.erase(endNode.k);
					_cacheList.pop_front();
				}
			}
#endif
			{
				boost::lock_guard<boost::mutex> guard(_cacheMtx); 
				_cacheList.push_front(Node(k, v));
				_keyMap[k] = _cacheList.begin();
			}

		}

		int hitRate(void) const
		{
			return _hitRage;
		}

		size_t reqCnt() const
		{
			return _reqCnt;
		}

		size_t hitCnt() const
		{
			return _cacheHitCnt;
		}

		size_t const allocatedSize()
		{
			return _cacheList.get_allocator().allocatedSize();
		}

		size_t const maxAllocSize() 
		{
			return _cacheList.get_allocator().max_size();
		}

	private:

		CacheList _cacheList;
		boost::unordered_map<__KEY_TYPE, typename std::list<Node>::iterator> _keyMap;	

		boost::atomic<size_t> _reqCnt;
		boost::atomic<size_t> _cacheHitCnt;
		boost::atomic<time_t> _reqCntStartTime;
		boost::atomic<int> _hitRage;
		std::list<double> _samples;
		boost::mutex _cacheMtx;
	};
}
#endif // LRUCACHE_H
