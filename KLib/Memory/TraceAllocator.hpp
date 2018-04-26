#include <memory>
#include <boost/atomic.hpp>

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

		template<class T>
		struct rebind {
			typedef TraceAllocator<T> other;
		};

		inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer p = 0)
		{
			_capacity.add(cnt, std::memory_order_release);
			return std::allocator<T>::allocate(cnt, p);
		}

		inline pointer allocate(size_type cnt)
		{
			_capacity.add(cnt, std::memory_order_release);
			return std::allocator<T>::allocate(cnt);
		}

		size_t const getCapacity()
		{
			return _capacity.load(std::memory_order_acquire);
		}		

	private:
		boost::atomic<size_t> _capacity;
	};

}
