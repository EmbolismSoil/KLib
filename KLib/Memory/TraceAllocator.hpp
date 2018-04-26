#ifndef __TRACEALLOCATOR_HPP__
#define __TRACEALLOCATOR_HPP__

#include <memory>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <new>

namespace KLib
{
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
			static __SingletonTraceAllocator allocator;
			return allocator;
		}

	private:
		boost::atomic<size_type> _capacity;
		__SingletonTraceAllocator() {}
		virtual ~__SingletonTraceAllocator() {}
	};

	template<class T, size_t max_alloc_size=std::numeric_limits<size_t>::max>
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

		inline TraceAllocator():_allocator(__SingletonTraceAllocator::instance())
		{

		}

		inline ~TraceAllocator(){}

		inline TraceAllocator(TraceAllocator const&): _allocator(__SingletonTraceAllocator::instance())
		{
		}

		template<typename U>
		inline TraceAllocator(TraceAllocator<U> const&):_allocator(__SingletonTraceAllocator::instance())
		{
		}

		template<class U>
		struct rebind {
			typedef TraceAllocator<U> other;
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
		
		size_type const max_size() 
		{
			return max_alloc_size;
		}
	private:
		__SingletonTraceAllocator& _allocator;
	};
}

#endif
