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
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <deque>
#include <queue>
#include <algorithm>
#include <iterator>
#include "utils/containerutils.hpp"


namespace KLib
{

	#define __DECL_CACHE_TYPE(_container) \
		template<class T, class Alloc> \
		struct cache_type<_container<T, Alloc> > \
		{\
			typedef _container<typename cache_type<T>::type, TraceAllocator<typename cache_type<T>::type> > type;\
		}

	#define __DECL_MAP_CACHE_TYPE(_map)\
		template<class KT, class VT, class Cmp, class Alloc>\
		struct cache_type<_map<KT, VT, Cmp, Alloc> >\
		{\
			typedef _map<typename cache_type<KT>::type, typename cache_type<VT>::type, Cmp, \
				TraceAllocator<std::pair<typename cache_type<KT>::type, typename cache_type<VT>::type> > > type;\
		}


	typedef std::basic_string<char, std::char_traits<char>, TraceAllocator<char> > cache_string;
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

	template<class T1, class T2>
	struct cache_type<std::pair<T1, T2> >
	{
		typedef std::pair<typename cache_type<T1>::type, typename cache_type<T2>::type> type;
	};

	__DECL_CACHE_TYPE(std::vector);
	__DECL_CACHE_TYPE(std::list);
	__DECL_MAP_CACHE_TYPE(std::map);
	__DECL_CACHE_TYPE(std::set);
	__DECL_MAP_CACHE_TYPE(std::multimap);
	__DECL_CACHE_TYPE(std::multiset);
	__DECL_CACHE_TYPE(std::stack);
	__DECL_CACHE_TYPE(std::deque);
	__DECL_CACHE_TYPE(std::priority_queue);

	template<class T, bool T_is_container>
	struct cache_type_converter;

	template<class T>
	typename cache_type<T>::type fromRaw(T const& raw)
	{
		return cache_type_converter<T, is_container<T>::value >::from(raw);
	}

	template<class T>
	struct cache_type_converter<T, false>
	{
		static typename cache_type<T>::type from(T const& raw)
		{
			return raw;
		}
	};

	template<>
	struct cache_type_converter<std::string, true>
	{
		static typename cache_type<std::string>::type from(const std::string &raw)
		{
			return typename cache_type<std::string>::type(raw.begin(), raw.end());
		}
	};

	template<class T1, class T2>
	struct cache_type_converter<std::pair<T1, T2>, false>
	{
		static typename cache_type<std::pair<T1, T2> >::type from(std::pair<T1, T2> const& raw)
		{
			typename cache_type<std::pair<T1, T2> >::type 
			cache_obj(cache_type_converter<T1, is_container<T1>::value>::from(raw.first), 
						cache_type_converter<T2, is_container<T2>::value>::from(raw.second));
			return cache_obj;
		}
	};

	template<class C>
	struct cache_type_converter<C, true>
	{
		static typename cache_type<C>::type from(const C &raw)
		{			
			typename cache_type<C>::type cache_obj;
			//typename __inserter<typename cache_type<C>::type>::inserter_iterator oiter(cache_obj);
			if (raw.empty())
			{
				return cache_obj;
			}
			
			std::transform(raw.begin(), raw.end(), inserter(cache_obj), 
				&cache_type_converter<typename C::value_type, is_container<typename C::value_type>::value >::from);
		}
	};

	template<class VT, bool VT_is_container>
	struct __LRUCacheNode;

	template<class VT>
	struct __LRUCacheNode<VT, false> 
	{
		cache_string k;
		typename cache_type<VT>::type v;
		__LRUCacheNode(std::string const& _k, VT const& _v) : k(_k.begin(), _k.end()), v(_v) {}
		__LRUCacheNode(cache_string const& _k, VT const& _v) : k(_k), v(_v) {}
	};

	template<class VT>
	struct __LRUCacheNode<VT, true>
	{
		cache_string k;
		typename cache_type<VT>::type v;
		__LRUCacheNode(std::string const& _k, VT const& _v): 
			k(_k.begin(), _k.end()), 
			v(fromRaw(_v))
		{

		}

		__LRUCacheNode(cache_string const& _k, typename cache_type<VT>::type const& _v):k(_k),v(_v){}

	};
	
	template<class __VALUE_TYPE, size_t max_alloc_size=KLib::__max_size_t_limit>
	class LRUCache : boost::noncopyable {
	public:
		typedef cache_string __KEY_TYPE;
		typedef __LRUCacheNode<__VALUE_TYPE, is_container<__VALUE_TYPE>::value> Node;
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

		void put(std::string const& k, __VALUE_TYPE const& v)
		{
			Node node(k, v);
			put(node);
		}

		void put(Node const& node)
		{
			{
				boost::lock_guard<boost::mutex> guard(_cacheMtx);

				typename boost::unordered_map<__KEY_TYPE, typename CacheList::iterator>::iterator pos;
				if ((pos = _keyMap.find(node.k)) != _keyMap.end()) {
					typename CacheList::iterator vit = pos->second;
					vit->v = node.v; //update
					_cacheList.splice(_cacheList.begin(), _cacheList, vit);
					_keyMap[node.k] = _cacheList.begin();
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
				_cacheList.push_front(node);
				_keyMap[node.k] = _cacheList.begin();
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
