#pragma once

#include "ForwardListConstIterator.h"

namespace volstl
{
	template<typename T>
	class ForwardListIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
		using Node = typename DataStructure::Node;
	public:
		ForwardListIterator() : m_Ptr(nullptr) {}
		ForwardListIterator(Node* nodePtr) : m_Ptr(nodePtr) {}
		~ForwardListIterator() {}

		operator ForwardListConstIterator<DataStructure>() const
		{
			return ForwardListConstIterator<DataStructure>(m_Ptr);
		}

		// 前缀运算符，++it
		ForwardListIterator& operator++()
		{
			m_Ptr = m_Ptr->next;
			return *this;
		}

		// 后缀运算符，it++
		ForwardListIterator operator++(int)
		{
			ForwardListIterator temp = *this;
			m_Ptr = m_Ptr->next;
			return temp;
		}

		ForwardListIterator& operator+=(const size_t count)
		{
			for (int i = 0; i != count; i++)
			{
				m_Ptr = m_Ptr->next;
				if (m_Ptr == nullptr)
					break;
			}
			return *this;
		}

		ForwardListIterator operator+(const size_t count) const
		{
			ForwardListIterator iterator = *this;
			iterator += count;
			return iterator;
		}

		int operator-(const ForwardListIterator& other) const
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

		bool operator==(const ForwardListIterator& other) const
		{
			return m_Ptr == other.m_Ptr;
		}

		bool operator!=(const ForwardListIterator& other) const
		{
			return !(*this == other);
		}

	private:
		// 前缀运算符，--it
		ForwardListIterator& operator--() = delete;
		// 后缀运算符，it--
		ForwardListIterator operator--(int) = delete;
		ForwardListIterator& operator-=(const int count) = delete;
		ForwardListIterator operator-(const int count) const = delete;
		bool operator<(const ForwardListIterator& other) const {}
		bool operator>(const ForwardListIterator& other) const {}
		bool operator<=(const ForwardListIterator& other) const {}
		bool operator>=(const ForwardListIterator& other) const {}


	public:
		Node* m_Ptr;
	};
}