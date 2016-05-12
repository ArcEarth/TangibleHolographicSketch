#pragma once

#ifndef STDX_STRIDE_RANGE
#define STDX_STRIDE_RANGE

#include <iterator>
#include <cstdint>
#include <type_traits>
#include "iterator_range.h"

namespace stdx {
	using std::iterator_range;

	template <class T, size_t _Stride>
	class stride_range;

	template <class _Ty, size_t _Stride>
	struct stride_iterator_storage
	{
		_Ty* data;
		constexpr static size_t stride = _Stride;

		stride_iterator_storage() { data = nullptr; }
		stride_iterator_storage(_Ty* _data, size_t _stride) : data(_data) {}

		inline constexpr size_t get_stride() const { return stride; }

		void assign(_Ty* _data, size_t _stride)
		{
			data = _data;
			assert(_stride == stride || _stride == -1);
		}

		template <size_t _OtherStride>
		void assign(const stride_iterator_storage<_Ty, _OtherStride>& other)
		{
			static_assert(_Stride == _OtherStride, "static stride storage must has _Equal_Stride_");
			data = other.data;
		}
	};

	template <class _Ty>
	struct stride_iterator_storage<_Ty, -1>
	{
		_Ty* data;
		size_t stride;

		stride_iterator_storage() { data = nullptr; stride = sizeof(_Ty); }
		stride_iterator_storage(_Ty* _data, size_t _stride) : data(_data), stride(_stride) {}

		inline size_t get_stride() const { return stride; }

		void assign(_Ty* _data, size_t _stride)
		{
			data = _data;
			stride = _stride;
			assert(_stride != -1);
		}

		template <size_t _OtherStride>
		void assign(const stride_iterator_storage<_Ty, _OtherStride>& other)
		{
			data = other.data;
			stride = other.get_stride();
		}

	};


	template <class _Ty, size_t _Stride = -1>
	class stride_iterator : 
		public std::iterator<std::random_access_iterator_tag, _Ty>,
		public stride_iterator_storage<_Ty,_Stride>
	{
		using byte_ptr = std::conditional_t<std::is_const<_Ty>::value, const char*, char*>;
		using storage_t = stride_iterator_storage<_Ty, _Stride>;

	protected:
		// these friend declarations are to allow access to the protected
		// "raw" constructor that starts from a raw pointer plus
		// stride+length info
		template <class U, size_t _Stride> friend class stride_iterator;
		template <class U, size_t _Stride> friend class stride_range;

		stride_iterator(pointer data, size_t stride = -1)
		{
			storage_t::assign(data, stride);
		}

	public:
		typedef stride_iterator<_Ty, _Stride> _Myiter;

		template <size_t _OtherStride>
		stride_iterator(const stride_iterator<_Ty, _OtherStride>& other)
		{
			storage_t::assign(other);
		}

		// Dereference
		reference operator*()
		{
			return *data;
		}
		reference operator[](int idx)
		{
			auto ptr = reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(data) + stride*idx);
			return *ptr;
		}

		// Comparison

		bool operator==(const _Myiter& other) const { return data == other.data; }

		bool operator!=(const _Myiter& other) const { return data != other.data; }

		bool operator<(const _Myiter& other) const { return data < other.data; }

		difference_type operator-(const _Myiter& other) const
		{
			return (reinterpret_cast<byte_ptr>(data)- reinterpret_cast<byte_ptr>(other.data)) / stride;
		}

		// Increment/Decrement

		_Myiter& operator++() { data = reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(data) + stride); return *this; }
		_Myiter& operator--() { data = reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(data) - stride); return *this; }

		_Myiter operator++(int) { _Myiter cpy(*this); data += stride; return cpy; }
		_Myiter operator--(int) { _Myiter cpy(*this); data -= stride; return cpy; }

		_Myiter& operator+=(int x) { data = &(*this)[x]; return *this; }
		_Myiter& operator-=(int x) { data = &(*this)[-x]; return *this; }

		_Myiter operator+(int x) const { _Myiter res(*this); res += x; return res; }
		_Myiter operator-(int x) const { _Myiter res(*this); res -= x; return res; }
	};
}

//namespace std {
//	template<class _Container, size_t _Stride>
//	struct _Is_checked_helper<stdx::stride_iterator<_Container, _Stride> >
//		: public true_type
//	{	// mark back_insert_iterator as checked
//	};
//}

namespace stdx {
	template <class _Ty, size_t _Stride = -1>
	class stride_range : protected stride_iterator_storage<_Ty,_Stride>
	{
	public:
		typedef	std::random_access_iterator_tag	iterator_category;
		//typedef stride_range<T>							_SelfType;
		typedef std::remove_reference_t<_Ty>					value_type;
		typedef ptrdiff_t										difference_type;
		typedef value_type*										pointer;
		typedef value_type&										reference;

		using byte_ptr = std::conditional_t<std::is_const<_Ty>::value, const char*, char*>;
		using storage_t = stride_iterator_storage<_Ty, _Stride>;

		typedef stride_iterator<value_type, _Stride>					iterator_type;
		typedef stride_iterator<std::add_const_t<value_type>, _Stride>	const_iterator_type;

	protected:
		pointer stop;
	public:
		stride_range()
			: stop(nullptr)
		{}

		stride_range(pointer _data, size_t _count, size_t _stride = -1)
			: storage_t(_data, _stride), stop(reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(_data) + stride*_count))
		{}

		void reset(pointer data, size_t count, size_t _stride = -1)
		{
			storage_t::assign(data, _stride);
			this->stop = reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(data) + stride*count);
		}

		void reset()
		{
			storage_t::assign(nullptr, -1);
			stop = nullptr;
		}

		bool empty() const
		{
			return stop - data == 0;
		}

		size_t size()
		{
			return ((byte_ptr) stop - (byte_ptr) data) / stride;
		}

		iterator_type begin()
		{
			return iterator_type(data, stride);
		}

		iterator_type end()
		{
			return iterator_type(stop, stride);
		}

		const_iterator_type begin() const
		{
			return const_iterator_type(data, stride);
		}

		const_iterator_type end() const
		{
			return const_iterator_type(stop, stride);
		}

		const_iterator_type cbegin() const
		{
			return const_iterator_type(data, stride);
		}

		const_iterator_type cend() const
		{
			return const_iterator_type(stop, stride);
		}

		reference operator[](int idx)
		{
			auto ptr = reinterpret_cast<pointer>(reinterpret_cast<byte_ptr>(data) + stride*idx);
#if _ITERATOR_DEBUG_LEVEL == 2
			if (stop <= ptr)
			{	// report error
				_DEBUG_ERROR("vector subscript out of range");
				_SCL_SECURE_OUT_OF_RANGE;
			}

#elif _ITERATOR_DEBUG_LEVEL == 1
			_SCL_SECURE_VALIDATE_RANGE(ptr < stop);
#endif /* _ITERATOR_DEBUG_LEVEL */
			return *ptr;
		}

		const reference operator[](int idx) const
		{
			auto ptr = reinterpret_cast<const pointer>(reinterpret_cast<const char*>(data) + stride*idx);
#if _ITERATOR_DEBUG_LEVEL == 2
			if (stop <= ptr)
			{	// report error
				_DEBUG_ERROR("vector subscript out of range");
				_SCL_SECURE_OUT_OF_RANGE;
			}

#elif _ITERATOR_DEBUG_LEVEL == 1
			_SCL_SECURE_VALIDATE_RANGE(ptr < stop);
#endif /* _ITERATOR_DEBUG_LEVEL */
			return *ptr;
		}

	};
}

#ifdef STDX_STRIDE_RANGE_IMPORT_TO_STD
namespace std
{
	using stdx::stride_iterator;
	using stdx::stride_range;
}
#endif

#endif