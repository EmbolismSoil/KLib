#ifndef __TRACEALLOCATOR_HPP__
#define __TRACEALLOCATOR_HPP__

#include <memory>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <new>
#include <limits.h>
#include <stdint.h>
#include <iostream>

namespace KLib
{
	static const uint64_t __max_size_t_limit(0xffffffffffffffff);

	class __SingletonTraceAllocator: boost::noncopyable
	{
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

		static __SingletonTraceAllocator& instance() 
		{
			static __SingletonTraceAllocator _instance;
			return _instance;
		}
#if 0
		__TraceAllocator(__TraceAllocator const& rhs) 
		{
			_capacity.store(rhs._capacity.load(boost::memory_order_acquire), boost::memory_order_release);
		}

		__TraceAllocator &operator=(__TraceAllocator const& rhs) 
		{
			_capacity.store(rhs._capacity.load(boost::memory_order_acquire), boost::memory_order_release);
		}
#endif
		virtual ~__SingletonTraceAllocator() {}

	private:
		boost::atomic<size_type> _capacity;
		__SingletonTraceAllocator() 
		{ 
			_capacity.store(0, boost::memory_order_release); 		
		}
	};

	template<class T, uint64_t __max_alloc_size= __max_size_t_limit>
	class TraceAllocator : public std::allocator<T>
	{
		template<class OT, uint64_t>
		friend class TraceAllocator;
	public:
		typedef typename std::allocator<T>::value_type value_type;
		typedef typename std::allocator<T>::pointer pointer;
		typedef typename std::allocator<T>::const_pointer const_pointer;
		typedef typename std::allocator<T>::reference reference;
		typedef typename std::allocator<T>::const_reference const_reference;
		typedef typename std::allocator<T>::size_type size_type;
		typedef typename std::allocator<T>::difference_type difference_type;
        typedef char char_type;

		static const uint64_t max_alloc_size;
		static const uint64_t item_size;

		inline TraceAllocator():
			_allocator(__SingletonTraceAllocator::instance())
		{

		}

		inline ~TraceAllocator(){}

		inline TraceAllocator(TraceAllocator const& oth):
			_allocator(__SingletonTraceAllocator::instance())
		{
			
		}

		template<typename U, uint64_t __u_max_alloc_size>
		inline TraceAllocator(TraceAllocator<U, __u_max_alloc_size> const& oth):
			_allocator(__SingletonTraceAllocator::instance())
		{

		}

		template<class U>
		struct rebind {
			typedef TraceAllocator<U, __max_alloc_size> other;
		};


		inline pointer allocate(size_type cnt)
		{
			pointer np = reinterpret_cast<pointer>(_allocator.allocate(cnt*sizeof(T)));
			return np;
		}



		inline size_type const allocatedSize() const
		{
			return _allocator.getCapacity();
		}
		
		size_type const max_size() const
		{
			return __max_alloc_size;
		}

	private:
		__SingletonTraceAllocator & _allocator;
	};

	template<class T, uint64_t __max_alloc_size>
	const uint64_t TraceAllocator<T, __max_alloc_size>::max_alloc_size(__max_alloc_size);

	template<class T, uint64_t __max_alloc_size>
	const uint64_t TraceAllocator<T, __max_alloc_size>::item_size(sizeof(T));
}

#endif
