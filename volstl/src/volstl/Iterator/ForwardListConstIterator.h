#pragma once

namespace volstl
{
	template<typename T>
	class ForwardListConstIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
		using Node = typename DataStructure::Node;
	public:
		ForwardListConstIterator() :m_Ptr() {}
		ForwardListConstIterator(Node* nodePtr) : m_Ptr(nodePtr) {}
		~ForwardListConstIterator() {}

		// 前缀运算符，++it
		ForwardListConstIterator& operator++()
		{
			m_Ptr = m_Ptr->next;
			return *this;
		}

		// 后缀运算符，it++
		ForwardListConstIterator operator++(int)
		{
			ForwardListConstIterator temp = *this;
			m_Ptr = m_Ptr->next;
			return temp;
		}

		ForwardListConstIterator& operator+=(const size_t count)
		{
			for (int i = 0; i != count; i++)
			{
				m_Ptr = m_Ptr->next;
				if (m_Ptr == nullptr)
					break;
			}
			return *this;
		}

		ForwardListConstIterator operator+(const size_t count) const
		{
			ForwardListConstIterator constIterator = *this;
			constIterator += count;
			return constIterator;
		}

		int operator-(const ForwardListConstIterator& other) const
		{
			int count = 0;
			for (Node* ptr = m_Ptr; ptr != other.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == nullptr)
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

		bool operator==(const ForwardListConstIterator& other) const
		{
			return m_Ptr == other.m_Ptr;
		}

		bool operator!=(const ForwardListConstIterator& other) const
		{
			return !(*this == other);
		}

	private:
		// 前缀运算符，--it
		ForwardListConstIterator& operator--() = delete;
		// 后缀运算符，it--
		ForwardListConstIterator operator--(int) = delete;
		ForwardListConstIterator& operator-=(const int count) = delete;
		ForwardListConstIterator operator-(const int count) const = delete;
		bool operator<(const ForwardListConstIterator& other) const {}
		bool operator>(const ForwardListConstIterator& other) const {}
		bool operator<=(const ForwardListConstIterator& other) const {}
		bool operator>=(const ForwardListConstIterator& other) const {}

	public:
		Node* m_Ptr;
	};
}