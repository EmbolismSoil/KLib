#ifndef __CONTAINERUTILS_HPP__
#define __CONTAINERUTILS_HPP__

#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <deque>
#include <queue>
#include <algorithm>
#include <iterator>

namespace KLib 
{
	#define __DECL_IS_CONTAINER(_container) \
	template<class T, class Alloc> \
	struct is_container<_container<T, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class T, class Alloc>\
	const bool is_container<_container<T, Alloc> >::value(true)

	#define __DECL_MAP_IS_CONTAINER(_map) \
	template<class KT, class VT, class Alloc> \
	struct is_container<_map<KT, VT, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class KT, class VT, class Alloc>\
	const bool is_container<_map<KT, VT, Alloc> >::value(true)\

	template<class T>
	struct is_associative_container 
	{
		static const bool value;
	};
	template<class T>
	const bool is_associative_container<T>::value(false);

	template<class T, class Cmp, class Alloc>
	struct is_associative_container<std::set<T, Cmp, Alloc> >
	{
		static const bool value;
	};
	template<class T, class Cmp, class Alloc>
	const bool is_associative_container<std::set<T, Cmp, Alloc> >::value(true);

	template<class T,class Cmp, class Alloc>
	struct is_associative_container<std::multiset<T, Cmp, Alloc> >
	{
		static const bool value;
	};
	template<class T, class Cmp, class Alloc>
	const bool is_associative_container<std::multiset<T, Cmp, Alloc> >::value(true);

	template<class K, class V, class Cmp, class Alloc>
	struct is_associative_container<std::map<K, V, Cmp, Alloc> >
	{
		static const bool value;
	};
	template<class K, class V, class Cmp, class Alloc>
	const bool is_associative_container<std::map<K, V, Cmp, Alloc> >::value(true);

	template<class K, class V, class Cmp, class Alloc>
	struct is_associative_container<std::multimap<K, V, Cmp, Alloc> >
	{
		static const bool value;
	};
	template<class K, class V, class Cmp, class Alloc>
	const bool is_associative_container<std::multimap<K, V, Cmp, Alloc> >::value(true);

	template<class T, bool is_associative_container>
	struct __inserter;

	template<class T>
	struct __inserter<T, false>
	{
		typedef std::back_insert_iterator<T> inserter_iterator;

	};

	template<class T>
	struct __inserter<T, true>
	{
		typedef std::insert_iterator<T> inserter_iterator;
	};	

	template<class T>
	typename __inserter<T, is_associative_container<T>::value>::inserter_iterator inserter(T & c)
	{
		return typename __inserter<T, is_associative_container<T>::value>::inserter_iterator(c);
	}


	template<class T>
	struct is_container
	{
		static const bool value;
	};

	template<class T>
	const bool is_container<T>::value(false);

	template<>
	struct is_container<std::string>
	{
		static const bool value;
	};

	const bool is_container<std::string>::value(true);

	__DECL_IS_CONTAINER(std::vector);
	__DECL_IS_CONTAINER(std::list);
	__DECL_MAP_IS_CONTAINER(std::map);
	__DECL_IS_CONTAINER(std::set);
	__DECL_MAP_IS_CONTAINER(std::multimap);
	__DECL_IS_CONTAINER(std::multiset);
	__DECL_IS_CONTAINER(std::stack);
	__DECL_IS_CONTAINER(std::deque);
	__DECL_IS_CONTAINER(std::priority_queue);

	template<class T>
	std::string container_to_string(T const& container);
}

#endif