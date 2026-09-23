#pragma once

namespace volstl
{
	template<typename T>
	class ListConstIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
		using Node = typename DataStructure::Node;
	public:
		ListConstIterator() :m_Ptr() {}
		ListConstIterator(Node* nodePtr) : m_Ptr(nodePtr) {}
		~ListConstIterator() {}

		// 前缀运算符，++it
		ListConstIterator& operator++()
		{
			m_Ptr = m_Ptr->next;
			return *this;
		}

		// 后缀运算符，it++
		ListConstIterator operator++(int)
		{
			ListConstIterator temp = *this;
			m_Ptr = m_Ptr->next;
			return temp;
		}

		// 前缀运算符，--it
		ListConstIterator& operator--()
		{
			m_Ptr = m_Ptr->prior;
			return *this;
		}

		// 后缀运算符，it--
		ListConstIterator operator--(int)
		{
			ListConstIterator temp = *this;
			m_Ptr = m_Ptr->prior;
			return temp;
		}

		ListConstIterator& operator+=(const int count)
		{
			if (count > 0)
				for (int i = 0; i != count; i++)
					m_Ptr = m_Ptr->next;
			else
				for (int i = count; i != 0; i++)
					m_Ptr = m_Ptr->prior;
			return *this;
		}

		ListConstIterator operator+(const int count) const
		{
			ListConstIterator constIterator = *this;
			constIterator += count;
			return constIterator;
		}

		ListConstIterator& operator-=(const int count)
		{
			if (count > 0)
				for (int i = 0; i != count; i++)
					m_Ptr = m_Ptr->prior;
			else
				for (int i = count; i != 0; i++)
					m_Ptr = m_Ptr->next;
			return *this;
		}

		ListConstIterator operator-(const int count) const
		{
			ListConstIterator constIterator = *this;
			constIterator -= count;
			return constIterator;
		}

		int operator-(const ListConstIterator& other) const
		{
			int count = 0;
			for (Node* ptr = m_Ptr; ptr != other.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == m_Ptr)
					return 0;
				count++;
			}
			return count;
		}

		ElementType& operator*() const
		{
			return m_Ptr->data;
		}

		ElementType* operator->() const
		{
			return &m_Ptr->data;
		}

		bool operator==(const ListConstIterator& other) const
		{
			return m_Ptr == other.m_Ptr;
		}

		bool operator!=(const ListConstIterator& other) const
		{
			return !(*this == other);
		}

	private:
		bool operator<(const ListConstIterator& other) const {}
		bool operator>(const ListConstIterator& other) const {}
		bool operator<=(const ListConstIterator& other) const {}
		bool operator>=(const ListConstIterator& other) const {}

	public:
		Node* m_Ptr;
	};
}