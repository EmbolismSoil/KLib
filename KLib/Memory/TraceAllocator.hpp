#ifndef __TRACEALLOCATOR_HPP__
#define __TRACEALLOCATOR_HPP__

#include <memory>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <new>

namespace KLib
{
	template<class T>
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

		inline explicit TraceAllocator()
		{
			__allocator = __SingletonTraceAllocator.instance();
		}

		inline ~TraceAllocator(){}

		inline explicit TraceAllocator(TraceAllocator const&oth): _allocator(oth._allocator)
		{

		}

		template<typename U>
		inline explicit TraceAllocator(TraceAllocator<U> const&oth):
			_allocator(oth._allocator)
		{
		
		}

		template<class T>
		struct rebind {
			typedef TraceAllocator<T> other;
		};

		inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer p = 0)
		{
			pointer np = reinterpret_cast<pointer>(_allocator.allocate(cnt));
			return np;
		}

		inline pointer allocate(size_type cnt)
		{
			pointer np = reinterpret_cast<pointer>(_allocator.allocate(cnt));
			return p;
		}

		inline size_type const allocatedSize()
		{
			return _allocator.getCapacity();
		}
	
	private:
		__SingletonTraceAllocator& _allocator;
	};

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

			_capacity.add(cnt, boost::memory_order_release);
			return p;
		}

		size_type const getCapacity()
		{
			return _capacity.load(boost::memory_order_acquire);
		}

		static __SingletonTraceAllocator& instance()
		{
			static __SingletonTraceAllocator allocator;
			return allocator;
		}

	private:
		boost::atomic<size_type> _capacity;
		__SingletonTraceAllocator() {}
		virtual ~__SingletonTraceAllocator() {}
	};

}

#endif