#ifndef __CONTAINERUTILS_HPP__
#define __CONTAINERUTILS_HPP__

#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <deque>
#include <string>
#include <queue>
#include <algorithm>
#include <iterator>
#include "String/string_converter.hpp"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include "stdint.h"
#include <iostream>

#define S(s) #s
#define __DECL_CONTAINER_FMT(_container, _fmt) \
template<class T, class Alloc>\
struct container_fmt<_container<T, Alloc> >\
{\
	static const std::string fmt;\
};\
template<class T, class Alloc>\
const std::string container_fmt<_container<T, Alloc> >::fmt(_fmt);\
template<class T, class Alloc>\
struct container_fmt<_container<T, Alloc> const>\
{\
	static const std::string fmt;\
};\
template<class T, class Alloc>\
const std::string container_fmt<_container<T, Alloc> const>::fmt(_fmt)

#define __DECL_SET_CONTAINER_FMT(_set, _fmt) \
template<class T, class Cmp, class Alloc> \
struct container_fmt<_set<T, Cmp, Alloc> > \
{\
	static const char * fmt;\
};\
template<class T, class Cmp, class Alloc> \
const char * container_fmt<_set<T, Cmp, Alloc> >::fmt(_fmt)

#define __DECL_MAP_CONTAINER_FMT(_map, _fmt) \
template<class K, class V, class Cmp, class Alloc> \
struct container_fmt<_map<K, V, Cmp, Alloc> > \
{\
	static const char * fmt; \
};\
template<class K, class V, class Cmp, class Alloc> \
const char * container_fmt<_map<K, V, Cmp, Alloc> >::fmt(_fmt)

#define __DECL_IS_CONTAINER(_container) \
	template<class T, class Alloc> \
	struct is_container<_container<T, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class T, class Alloc> \
	const bool is_container<_container<T, Alloc> >::value(true); \
	template<class T, class Alloc> \
	struct is_container<_container<T, Alloc> const> \
	{\
		static const bool value;\
	};\
	template<class T, class Alloc> \
	const bool is_container<_container<T, Alloc> const>::value(true)

#define __DECL_MAP_IS_CONTAINER(_map) \
	template<class KT, class VT, class Cmp, class Alloc> \
	struct is_container<_map<KT, VT, Cmp, Alloc> > \
	{\
		static const bool value;\
	};\
	template<class KT, class VT, class Cmp, class Alloc> \
	const bool is_container<_map<KT, VT, Cmp, Alloc> >::value(true); \
	template<class KT, class VT, class Cmp, class Alloc> \
	struct is_container<_map<KT, VT, Cmp, Alloc> const> \
	{\
		static const bool value;\
	};\
	template<class KT, class VT, class Cmp, class Alloc> \
	const bool is_container<_map<KT, VT, Cmp, Alloc> const>::value(true)

#define __DECL_SET_IS_CONTAINER(_set) \
template<class T, class Cmp, class Alloc>	\
struct is_container<_set<T, Cmp, Alloc> > \
{\
	static const bool value; \
};\
template<class T, class Cmp, class Alloc> \
const bool is_container<_set<T, Cmp, Alloc> >::value(true); \
template<class T, class Cmp, class Alloc>	\
struct is_container<_set<T, Cmp, Alloc> const> \
{\
	static const bool value; \
};\
template<class T, class Cmp, class Alloc> \
const bool is_container<_set<T, Cmp, Alloc> const>::value(true)


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
	enum { value = sizeof(__has<T>(0)) == sizeof(uint8_t) };\
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

	template<>
	struct is_container<std::string const>
	{
		static const bool value;
	};
	const bool is_container<std::string const>::value(true);

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
	__DECL_CONTAINER_FMT(std::pair, "(%s)");

/*------------------------------  end container_fmt  --------------------------------------*/


/*------------------------------   has_value_type    --------------------------------------*/	
	__DECL_HAS_MEMBER_TYPE(value_type);
	__DECL_HAS_MEMBER_TYPE(mapped_type);

/*-------------------------------end has_value_type ---------------------------------------*/

template<class T>
struct has_to_string 
{
private:
	template<class U, std::string(U::* ) () const> struct __Helper;

	template<class U>
	static uint8_t __has(__Helper<U, &U::to_string> *) {}

	template<class U>
	static uint16_t __has(...) {}

public:
	enum {value=sizeof(__has<T>(0)) == sizeof(uint8_t)};
};

template<class T>
struct has_toString
{
private:
	template<class U, std::string(U::*)() const> struct __Helper;
	
	template<class U>
	static uint8_t __has(__Helper<U, &U::toString> *){}

	template<class U>
	static uint16_t __has(...){}

public:
	enum{value=sizeof(__has<T>(0)) == sizeof(uint8_t)};
};

template<class T>
struct has_to_string_or_toString {
	enum{value=has_toString<T>::value||has_to_string<T>::value};
};

/*------------------------------------item_type-----------------------------------------------*/
template<class T, bool has_mapped_type>
struct item_type;

template<class T>
struct item_type<T, true> 
{
	typedef typename T::mapped_type type;
};

template<class T>
struct item_type<T, false>
{
	typedef typename T::value_type type;
};

template<class T>
T strto(std::string const& s) 
{
	std::istringstream ss(s);
	T num;
	ss >> num;
	return num;
}

template<class T, bool is_container, bool has_to_string>
struct __ToString;
template<class T>
struct __ToString<T, false, false>
{
	static std::string to(T const& c)
	{
		std::stringstream ss;
		ss << c;
		return ss.str();
	}
};

template<>
struct __ToString<std::string const, true, false>
{
	static std::string to(std::string const& s)
	{
		boost::format fmt("\"%s\"");
		fmt % s;
		return fmt.str();
	}
};

template<>
struct __ToString<std::string, true, false>
{
	static std::string to(std::string const& s)
	{
		boost::format fmt("\"%s\"");
		fmt % s;
		return fmt.str();
	}
};

template<>
struct __ToString<char const*, false, false>
{
	static std::string to(char const* s)
	{
		if (!s)
		{
			return "\"\"";
		}
		boost::format fmt("\"%s\"");
		fmt % s;
		return fmt.str();
	}
};

template<>
struct __ToString<char *, false, false>
{
	static std::string to(char const* s)
	{
		boost::format fmt("\"%s\"");
		fmt % s;
		return fmt.str();
	}
};

template<class T>
struct __ToString<T, true, false> 
{
	static std::string to(T const& c) 
	{
		typedef typename T::value_type itemtype;
		enum{item_is_container=is_container<itemtype>::value};
		enum{item_has_to_string=has_to_string_or_toString<itemtype>::value};
		typedef __ToString<itemtype, item_is_container, item_has_to_string> __ItemToString;

		std::vector<std::string> buf;
		for (typename T::const_iterator pos = c.begin(); pos != c.end(); ++pos) 
		{
			itemtype const& item = *pos;
			buf.push_back(__ItemToString::to(item));				
		}

		std::string joined = boost::join(buf, ", ");
		boost::format fmt(container_fmt<T>::fmt);
		fmt % joined;
		return fmt.str();
	}
};

template<class T>
struct __ToString<T, false, true>
{
private:		
	template<class U, bool U_has_to_string>
	struct __Helper;

	template<class U>
	struct __Helper<U, true>
	{
		static std::string __to(U const& c)
		{
			return c.to_string();
		}
	};

	template<class U>
	struct __Helper<U, false>
	{
		static std::string __to(U const& c)
		{
			return c.toString();
		}
	};

public:		
	static std::string to(T const& c)
	{
		return __Helper<T, has_to_string<T>::value>::__to(c);
	}
};

template<class T1, class T2>
struct __ToString<std::pair<T1, T2>, false, false>
{
	static std::string to(std::pair<T1, T2> const& c)
	{
		static const bool T1_is_container = is_container<T1>::value;
		static const bool T2_is_container = is_container<T2>::value;
		static const bool T1_has_to_string = has_to_string<T1>::value || has_toString<T1>::value;
		static const bool T2_has_to_string = has_to_string<T2>::value || has_toString<T2>::value;

		typedef __ToString<T1, T1_is_container, T1_has_to_string> T1ToString;
		typedef __ToString<T2, T2_is_container, T2_has_to_string> T2ToString;
		
		boost::format joined_fmt("%s : %s");
		joined_fmt % T1ToString::to(c.first) % T2ToString::to(c.second);
		
		boost::format fmt(container_fmt<std::pair<T1, T2> >::fmt);
		fmt % joined_fmt.str();
		return fmt.str();
	}
};


template<class T>
std::string tostr(T const& c)
{
	return __ToString<T, is_container<T>::value, has_to_string<T>::value || has_toString<T>::value>::to(c);
}

}
#endif