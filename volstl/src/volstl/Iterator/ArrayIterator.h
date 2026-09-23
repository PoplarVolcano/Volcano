#pragma once

#include "ArrayConstIterator.h"

namespace volstl
{
	template<typename T>
	class ArrayIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
	public:
		ArrayIterator() : m_Ptr(nullptr) {}
		ArrayIterator(ElementType* ptr) : m_Ptr(ptr){}
		~ArrayIterator() {}

		// 只能Iterator转ConstIterator
		operator ArrayConstIterator<DataStructure>() const 
		{ 
			return ArrayConstIterator<DataStructure>(m_Ptr);
		}

		// 前缀运算符，++it
		ArrayIterator& operator++()
		{
			m_Ptr++;
			return *this;
		}

		// 后缀运算符，it++
		ArrayIterator operator++(int)
		{
			ArrayIterator iterator = *this;
			++(*this);
			return iterator;
		}

		// 前缀运算符，--it
		ArrayIterator& operator--()
		{
			m_Ptr--;
			return *this;
		}

		// 后缀运算符，it--
		ArrayIterator operator--(int)
		{
			ArrayIterator iterator = *this;
			--(*this);
			return iterator;
		}

		ArrayIterator& operator+=(const int count)
		{
			m_Ptr += count;
			return *this;
		}

		ArrayIterator operator+(const int count) const
		{
			ArrayIterator iterator = *this;
			iterator.m_Ptr += count;
			return iterator;
		}

		ArrayIterator& operator-=(const int count)
		{
			m_Ptr -= count;
			return *this;
		}

		ArrayIterator operator-(const int count) const
		{
			ArrayIterator iterator = *this;
			iterator.m_Ptr -= count;
			return iterator;
		}

		int operator-(const ArrayIterator& other) const
		{
			return m_Ptr - other.m_Ptr;
		}

		ElementType& operator[](int index)
		{
			return *(m_Ptr + index);
		}

		const ElementType& operator[](int index) const
		{
			return *(m_Ptr + index);
		}

		ElementType* operator->()
		{
			return m_Ptr;
		}

		const ElementType* operator->() const
		{
			return m_Ptr;
		}

		ElementType& operator*()
		{
			return *m_Ptr;
		}

		const ElementType& operator*() const
		{
			return *m_Ptr;
		}


		bool operator==(const ArrayIterator& other) const
		{
			return m_Ptr == other.m_Ptr;
		}

		bool operator!=(const ArrayIterator& other) const		
		{
			return m_Ptr != other.m_Ptr;
		}

		bool operator<(const ArrayIterator& other) const 
		{
			return m_Ptr < other.m_Ptr;
		}

		bool operator>(const ArrayIterator& other) const 
		{
			return m_Ptr > other.m_Ptr;
		}

		bool operator<=(const ArrayIterator& other) const 
		{
			return m_Ptr <= other.m_Ptr;
		}

		bool operator>=(const ArrayIterator& other) const 
		{
			return m_Ptr >= other.m_Ptr;
		}



	public:
		ElementType* m_Ptr;
	};


}