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
#include "String/string_converter.hpp"
#include <type_traits>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "stdint.h"

#define __DECL_CONTAINER_FMT(_container, _fmt) \
template<class T, class Alloc>\
struct container_fmt<_container<T, Alloc> >\
{\
	static const char const* fmt;\
};\
template<class T, class Alloc>\
const char const* container_fmt<_container<T, Alloc> >::fmt(_fmt)

#define __DECL_SET_CONTAINER_FMT(_set, _fmt) \
template<class T, class Cmp, class Alloc> \
struct container_fmt<_set<T, Cmp, Alloc> > \
{\
	static const char const* fmt;\
};\
template<class T, class Cmp, class Alloc> \
const char const* container_fmt<_set<T, Cmp, Alloc> >::fmt(_fmt)

#define __DECL_MAP_CONTAINER_FMT(_map, _fmt) \
template<class K, class V, class Cmp, class Alloc> \
struct container_fmt<_map<K, V, Cmp, Alloc> > \
{\
	static const char const* fmt; \
};\
template<class K, class V, class Cmp, class Alloc> \
const char const* container_fmt<_map<K, V, Cmp, Alloc> >::fmt(_fmt)

#define __DECL_IS_CONTAINER(_container) \
	template<class T, class Alloc> \
	struct is_container<_container<T, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class T, class Alloc>\
	const bool is_container<_container<T, Alloc> >::value(true)

#define __DECL_MAP_IS_CONTAINER(_map) \
	template<class KT, class VT, class Cmp, class Alloc> \
	struct is_container<_map<KT, VT, Cmp, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class KT, class VT, class Cmp, class Alloc>\
	const bool is_container<_map<KT, VT, Cmp, Alloc> >::value(true)\

#define __DECL_SET_IS_CONTAINER(_set) \
template<class T, class Cmp, class Alloc>	\
struct is_container<_set<T, Cmp, Alloc> > \
{\
	static const bool value; \
};\
template<class T, class Cmp, class Alloc> \
const bool is_container<_set<T, Cmp, Alloc> >::value(true)


#define __DECL_HAS_MEMBER_TYPE(_member) \
template<class T>\
struct has_##_member\
{\
private:\
	template<class U>\
	static const uint8_t __has(typename U::_member *) {}\
\
	template<class U>\
	static const uint16_t __has(...) {}\
\
public:\
	enum { Yes = sizeof(__has<T>(0)) == sizeof(uint8_t) };\
}

namespace KLib
{
/*--------------------------------is_associative_container------------------------------------*/
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

/*--------------------------  end is_associative_container  ----------------------------------*/

/*----------------------------------  inserter  ----------------------------------------------*/
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
/*----------------------------------  end inserter  -----------------------------------------*/

/*----------------------------------  is_container  -----------------------------------------*/
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
	__DECL_SET_IS_CONTAINER(std::set);
	__DECL_MAP_IS_CONTAINER(std::multimap);
	__DECL_SET_IS_CONTAINER(std::multiset);
	__DECL_IS_CONTAINER(std::stack);
	__DECL_IS_CONTAINER(std::deque);
	__DECL_IS_CONTAINER(std::priority_queue);

/*--------------------------------  end is_container  ---------------------------------------*/

/*--------------------------------  container_fmt  ----------------------------------------*/
	template<class T>
	struct container_fmt;

	__DECL_CONTAINER_FMT(std::vector, "[%s]");
	__DECL_CONTAINER_FMT(std::list, "[%s]");
	__DECL_MAP_CONTAINER_FMT(std::map, "{%s}");
	__DECL_MAP_CONTAINER_FMT(std::multimap, "{%s}");
	__DECL_SET_CONTAINER_FMT(std::set, "(%s)");
	__DECL_SET_CONTAINER_FMT(std::multiset, "(%s)");
	__DECL_CONTAINER_FMT(std::stack, "[%s]");
	__DECL_CONTAINER_FMT(std::deque, "[%s]");
	__DECL_CONTAINER_FMT(std::priority_queue, "[%s]");

/*------------------------------  end container_fmt  --------------------------------------*/


/*------------------------------   has_value_type    --------------------------------------*/	
	__DECL_HAS_MEMBER_TYPE(value_type);
	__DECL_HAS_MEMBER_TYPE(mapped_type);

/*-------------------------------end has_value_type ---------------------------------------*/

template<class T>
struct has_to_string 
{
private:
	template<class U, std::string(U::*)()> __Helper;

	template<class U>
	static uint8_t __has(__Helper<U, &U::to_string> *) {}

	template<class U>
	static uint16_t __has(...) {}

public:
	enum {value=sizeof(__has<T>(0)) == sizeof(uint8_t)};
};

/*------------------------------------item_type-----------------------------------------------*/
template<class T, bool has_mapped_type>
struct item_type;

template<class T>
struct item_type<true> 
{
	typedef T::mapped_type type;
};

template<class T>
struct item_type<false>
{
	typedef T::value_type type;
};

/*-------------------------------    container to string    -------------------------------*/
	template<class T, bool is_container, bool has_to_string>
	struct __ToString;

	template<class T, bool has_to_string>
	struct __ToString<T, true, has_to_string> 
	{
		template<bool has_value_type, bool has_mapped_type>
		static std::string to();

		std::string to(T const& c) 
		{
			typedef item_type<T, has_mapped_type<T>::value>::type itemtype;
			static const bool item_is_container(is_container<itemtype>::value);
			static const bool item_has_to_string(has_to_string<item_type>::value);
			
			typedef __ToString<itemtype, item_is_container, item_has_to_string> __ItemToString;

			std::vector<std::string> buf;
			for (T::const_iterator pos = c.begin(); pos != c.end(); ++pos) 
			{
				itemtype const& item = *pos;
				buf.push_back(__ItemToString:to(item));				
			}

			std::string joined = boost::join(buf, ",");
			boost::format fmt(container_fmt<T>::fmt);
			fmt % joined;
			return fmt.str();
		}
	};


	class __ContainerToStringFunctor {
	public:
		__ContainerToStringFunctor(std::string const& seq, std::string const& fmt) :
			_seq(seq),
			_fmt(fmt)
		{

		}

		template<class T>
		void operator()(T const& item)
		{
			std::string tmp;
			std::stringstream ss;
			ss << item;
			ss >> tmp;
			_buf.push_back(tmp);
		}

		void operator()(std::string const& item)
		{
			boost::format fmt("'%s'");
			fmt % item;
			_buf.push_back(fmt.str());
		}

		std::string get()
		{
			boost::format fmt(_fmt);
			std::string joined = boost::join(_buf, _seq);
			fmt % joined;
			return fmt.str();
		}

	private:
		std::vector<std::string> _buf;
		std::string const _seq;
		std::string const _fmt;
	};
}

#endif