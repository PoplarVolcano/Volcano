#pragma once

#include "volstl/ForwardList.h"

namespace volstl
{
	// 栈，使用ForwardList（有头结点单链表）
	template<typename T>
	class Stack
	{
	public:
		using ElementType = T;

	public:
		Stack() {}
		~Stack() {}

		void Push(const ElementType& value)
		{
			m_List.EmplaceFront(value);
		}

		void Push(ElementType&& value)
		{
			m_List.EmplaceFront(std::forward<ElementType>(value));
		}

		void Pop()
		{
			m_List.EraseAfter(m_List.BeforeBegin());
		}
		
		ElementType& Top()
		{
			return m_List.Front();
		}

		bool Empty()
		{
			return m_List.Empty();
		}
		size_t Size()
		{
			return m_List.Size();
		}

	private:
		Stack(const Stack& other) {}
		Stack(Stack&& other) {}
		Stack& operator=(const Stack& other) {}
		Stack& operator=(Stack&& other) {}

	private:
		ForwardList<ElementType> m_List;
	};
}