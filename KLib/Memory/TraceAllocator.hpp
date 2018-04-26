#ifndef __TRACEALLOCATOR_HPP__
#define __TRACEALLOCATOR_HPP__

#include <memory>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <new>
#include <limits.h>
#include <stdint.h>

namespace KLib
{
	template<class T, uint64_t>
	class TraceAllocator;

	static const uint64_t __max_size_t_limit(0xffffffffffffffff);

	class __TraceAllocator
	{
		template<class T, uint64_t>
		friend class TraceAllocator;

	public:
		typedef size_t size_type;
		inline void* allocate(size_type cnt)
		{
			void *p = ::operator new(cnt);
			if (!p) {
				throw std::bad_alloc();
			}

			_capacity.fetch_add(cnt, boost::memory_order_release);
			return p;
		}

		size_type const getCapacity()
		{
			return _capacity.load(boost::memory_order_acquire);
		}

		__TraceAllocator() {}
		virtual ~__TraceAllocator() {}

	private:
		boost::atomic<size_type> _capacity;
	};

	template<class T, uint64_t __max_alloc_size= __max_size_t_limit>
	class TraceAllocator : public std::allocator<T>
	{
	public:
		typedef typename std::allocator<T>::value_type value_type;
		typedef typename std::allocator<T>::pointer pointer;
		typedef typename std::allocator<T>::const_pointer const_pointer;
		typedef typename std::allocator<T>::reference reference;
		typedef typename std::allocator<T>::const_reference const_reference;
		typedef typename std::allocator<T>::size_type size_type;
		typedef typename std::allocator<T>::difference_type difference_type;

		static const uint64_t max_alloc_size;

		inline TraceAllocator()
		{

		}

		inline ~TraceAllocator(){}

		inline TraceAllocator(TraceAllocator const& oth)
		{
		}

		template<typename U, uint64_t __u_max_alloc_size>
		inline TraceAllocator(TraceAllocator<U, __u_max_alloc_size> const& oth)
		{

		}

		template<class U>
		struct rebind {
			typedef TraceAllocator<U, __max_alloc_size> other;
		};


		inline pointer allocate(size_type cnt)
		{
			pointer np = reinterpret_cast<pointer>(_allocator.allocate(cnt));
			return np;
		}

		inline size_type const allocatedSize()
		{
			return _allocator.getCapacity();
		}
		
		size_type const max_size() const
		{
			return __max_alloc_size;
		}

	private:
		__TraceAllocator _allocator;
	};

	template<class T, uint64_t __max_alloc_size>
	const uint64_t TraceAllocator<T, __max_alloc_size>::max_alloc_size(__max_alloc_size);
}

#endif
