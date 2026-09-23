#pragma once

namespace volstl
{
	template<typename T>
	class ArrayConstIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
	public:
		ArrayConstIterator() : m_Ptr(nullptr) {}
		ArrayConstIterator(const ElementType* ptr) : m_Ptr(ptr) {}
		~ArrayConstIterator() {}

		// 前缀运算符，++it
		ArrayConstIterator& operator++()
		{
			m_Ptr++;
			return *this;
		}

		// 后缀运算符，it++
		ArrayConstIterator operator++(int)
		{
			ArrayConstIterator constIterator = *this;
			++(*this);
			return constIterator;
		}

		// 前缀运算符，--it
		ArrayConstIterator& operator--()
		{
			m_Ptr--;
			return *this;
		}

		// 后缀运算符，it--
		ArrayConstIterator operator--(int)
		{
			ArrayConstIterator constIterator = *this;
			--(*this);
			return constIterator;
		}

		ArrayConstIterator& operator+=(const int count)
		{
			m_Ptr += count;
			return *this;
		}

		ArrayConstIterator operator+(const int count) const
		{
			ArrayConstIterator constIterator = *this;
			constIterator.m_Ptr += count;
			return constIterator;
		}

		ArrayConstIterator& operator-=(const int count)
		{
			m_Ptr -= count;
			return *this;
		}

		ArrayConstIterator operator-(const int count) const
		{
			ArrayConstIterator constIterator = *this;
			constIterator.m_Ptr -= count;
			return constIterator;
		}

		int operator-(const ArrayConstIterator& other) const
		{
			return m_Ptr - other.m_Ptr;
		}

		const ElementType& operator[](int index) const
		{
			return *(m_Ptr + index);
		}

		const ElementType* operator->() const
		{
			return m_Ptr;
		}

		const ElementType& operator*() const
		{
			return *m_Ptr;
		}


		bool operator==(const ArrayConstIterator& other) const
		{
			return m_Ptr == other.m_Ptr;
		}

		bool operator!=(const ArrayConstIterator& other) const
		{
			return m_Ptr != other.m_Ptr;
		}

		bool operator<(const ArrayConstIterator& other) const
		{
			return m_Ptr < other.m_Ptr;
		}

		bool operator>(const ArrayConstIterator& other) const
		{
			return m_Ptr > other.m_Ptr;
		}

		bool operator<=(const ArrayConstIterator& other) const
		{
			return m_Ptr <= other.m_Ptr;
		}

		bool operator>=(const ArrayConstIterator& other) const
		{
			return m_Ptr >= other.m_Ptr;
		}


	public:
		const ElementType* m_Ptr;
	};


}