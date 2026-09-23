#pragma once

#include "volstl/Iterator/ForwardListIterator.h"
#include "volstl/BlockAllocator.h"

namespace volstl
{
	template<typename T>
	struct FLNode
	{
		T data;
		FLNode* next;
	};

	// 有头结点单链表
	template<typename T>
	class ForwardList
	{
	public:
		using ElementType = T;
		using Node = FLNode<ElementType>;
		using Iterator = ForwardListIterator<ForwardList<ElementType>>;
		using ConstIterator = ForwardListConstIterator<ForwardList<ElementType>>;
	public:
		ForwardList() 
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node))), m_Rear(m_Front)
		{
			m_Front->next = nullptr;
		}

		~ForwardList()
		{
			Clear();
			//m_Front->~Node(); 头结点只申请内存，未执行构造，不需要执行析构
			volstl::BlockAllocator::GetInstance()->Free(m_Front, sizeof(Node));
		}

		ForwardList(size_t count, const ElementType& value)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node))), m_Rear(m_Front)
		{
			m_Front->next = nullptr;
			for (size_t i = 0; i < count; i++)
				EmplaceFront(value);
		}

		ForwardList(size_t count)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node))), m_Rear(m_Front)
		{
			m_Front->next = nullptr;
			ElementType temp;
			for (size_t i = 0; i < count; i++)
				EmplaceFront(temp);
		}

		// [first, last)
		ForwardList(ConstIterator first, ConstIterator last)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node))), m_Rear(m_Front)
		{
			m_Front->next = nullptr;
#if _DEBUG > 0
			if (first == last || first.m_Ptr == nullptr)
				assert("ForwardList: Invalid Iterator: first");

			// 检测first和last是否在同一容器中
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == nullptr)
					assert("ForwardList: Invalid Iterator: first, last");
			}
#endif
			Iterator tempIt = BeforeBegin();
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				EmplaceAfter(tempIt, ptr->data);
				tempIt++;
			}
		}

		template<typename U>
		ForwardList(std::initializer_list<U> initList)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node))), m_Rear(m_Front)
		{
			m_Front->next = nullptr;
			Iterator tempIt = BeforeBegin();
			for (auto it = initList.begin(); it != initList.end(); it++)
			{
				EmplaceAfter(tempIt, static_cast<std::remove_const_t<U>&&>(const_cast<U&>(*it)));
				tempIt++;
			}
		}

		Iterator InsertAfter(ConstIterator where, const ElementType& value)
		{
			if (where == end())
				return end();

			Node* newNode = (Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node));
			new(&newNode->data) ElementType(value);

			newNode->next = where.m_Ptr->next;
			where.m_Ptr->next = newNode;

			if (newNode->next == nullptr)
				m_Rear = newNode;

			return Iterator(newNode);

		}

		Iterator InsertAfter(ConstIterator where, ElementType&& value)
		{
			if (where == end())
				return end();

			Node* newNode = (Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node));
			new(&newNode->data) ElementType(std::forward<ElementType>(value));

			newNode->next = where.m_Ptr->next;
			where.m_Ptr->next = newNode;

			if (newNode->next == nullptr)
				m_Rear = newNode;

			return Iterator(newNode);
		}

		Iterator PushFront(const ElementType& value)
		{
			return InsertAfter(BeforeBegin(), value);
		}

		Iterator PushFront(ElementType&& value)
		{
			return InsertAfter(BeforeBegin(), std::forward<ElementType>(value));
		}

		template<typename... Args>
		Iterator EmplaceAfter(ConstIterator where, Args&&... args)
		{
			if (where == end())
				return end();

			Node* newNode = (Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node));
			new(&newNode->data) ElementType(std::forward<Args>(args)...);

			newNode->next = where.m_Ptr->next;
			where.m_Ptr->next = newNode;

			if (newNode->next == nullptr)
				m_Rear = newNode;

			return Iterator(newNode);

		}

		template<typename... Args>
		Iterator EmplaceFront(Args&&... args)
		{
			return EmplaceAfter(BeforeBegin(), std::forward<Args>(args)...);
		}

		Iterator EraseAfter(ConstIterator where)
		{
#if _DEBUG > 0
			if(where.m_Ptr == nullptr)
				assert("ForwardList::EraseAfter: Invalid Iterator: where");

			// 检验where是否在forwardList内
			for (ConstIterator it = BeforeBegin(); it != where; it++)
			{
				if (it == end())
					assert("ForwardList::EraseAfter: Invalid Iterator: where");
			}

			if (where.m_Ptr == m_Rear)
				assert("ForwardList::EraseAfter: Invalid Iterator: where");
#endif
			Node* temp        = where.m_Ptr->next;
			where.m_Ptr->next = where.m_Ptr->next->next;
			temp->~Node();
			volstl::BlockAllocator::GetInstance()->Free(temp, sizeof(Node));

			if (where.m_Ptr->next == nullptr)
				m_Rear = where.m_Ptr;

			return Iterator(where.m_Ptr);
		}

		// 区间删除，(first, last)
		Iterator EraseAfter(ConstIterator first, ConstIterator last)
		{
#if _DEBUG > 0
			// 表空；开始结点为表尾；结束结点为空
			if (Empty() || first == last || first.m_Ptr == m_Rear)
				assert("ForwardList::EraseAfter: Invalid Iterator");

			// 检验last是否在first之后
			for (ConstIterator it = first + 1; it != last; it++)
			{
				if (it == end())
					assert("ForwardList::EraseAfter: Invalid Iterator");
			}
#endif
			while (first + 1 != last)
				EraseAfter(first);

			return Iterator(first.m_Ptr);
		}

		void PopFront()
		{
			EraseAfter(BeforeBegin());
		}

		// 销毁 vector 中所有元素，并用value填充
		void Assign(size_t count, const ElementType& value)
		{
			Clear();
			for (size_t i = 0; i != count; i++)
				EmplaceAfter(Iterator(m_Rear), value);
		}

		// 销毁 vector 中所有元素，并用迭代器范围 [first, last) 的内容填充
		void Assign(Iterator first, Iterator last)
		{
#if _DEBUG > 0
			if (first == last || first.m_Ptr == nullptr)
				assert("ForwardList::Assign: Invalid Iterator");

			// 检测first和last是否在同一容器中
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == nullptr)
					assert("ForwardList::Assign: Invalid Iterator");
			}
#endif
			Clear();
			for (; first != last; first++)
				EmplaceAfter(Iterator(m_Rear), *first);
		}
		void Swap(ForwardList<ElementType>& other)
		{
			if (m_Front == other.m_Front)
				return;

			Node* front = other.m_Front;
			Node* rear  = other.m_Rear;
			other.m_Front = m_Front;
			other.m_Rear  = m_Rear;
			m_Front = front;
			m_Rear  = rear;
		}

		void Clear()
		{
			while (!Empty())
			{
				EraseAfter(BeforeBegin());
			}
		}

		void Resize(size_t count)
		{
			Iterator tempIt = BeforeBegin();
			for (size_t i = 0; i != count; i++)
			{
				tempIt++;
				if (tempIt == end())
					tempIt = EmplaceAfter(BeforeEnd());
			}
			if (tempIt + 1 != end())
			{
				for (ConstIterator it = tempIt; it + 1 != end();)
					EraseAfter(it);
			}
		}

		void SpliceAfter(ConstIterator pos, ForwardList& other)
		{

#if _DEBUG > 0
			if (pos.m_Ptr == nullptr)
				assert("ForwardList::EraseAfter: Invalid Iterator: where");

			// 检验where是否在forwardList内
			for (ConstIterator it = BeforeBegin(); it != pos; it++)
			{
				if (it == end())
					assert("ForwardList::EraseAfter: Invalid Iterator: where");
			}
#endif
			if (m_Rear == pos.m_Ptr)
				m_Rear = other.m_Rear;
			pos.m_Ptr->next     = other.m_Front->next;
			other.m_Front->next = nullptr;
			other.m_Rear        = other.m_Front;
		}

		void Merge(ForwardList& other)
		{
			// TODO
		}

		void Sort()
		{
			// TODO
		}

		void Reverse()
		{
			// TODO
		}

		void Remove(const ElementType& value)
		{
			for (Iterator it = BeforeBegin(); it != end();)
			{
				if (*it == value)
					it = EraseAfter(it);
				else
					it++;
			}

		}
		void RemoveIf(const ElementType& value)
		{

		}

		ElementType& Front()
		{
			return m_Front->next->data;
		}

		ElementType& Back()
		{
			return m_Rear->data;
		}

		const size_t Size() const
		{
			size_t size = 0;
			for (const auto& node : *this)
				size++;
			return size;
		}

		const bool Empty() const
		{
			return m_Front->next == nullptr;
		}

		Iterator BeforeBegin()
		{
			return Iterator(m_Front);
		}

		ConstIterator BeforeBegin() const
		{
			return ConstIterator(m_Front);
		}

		Iterator BeforeEnd()
		{
			return Iterator(m_Rear);
		}

		ConstIterator BeforeEnd() const
		{
			return ConstIterator(m_Rear);
		}

		Iterator begin()
		{
			return Iterator(m_Front->next);
		}

		ConstIterator begin() const
		{
			return ConstIterator(m_Front->next);
		}

		Iterator end()
		{
			return Iterator(nullptr);
		}

		ConstIterator end() const
		{
			return ConstIterator(nullptr);
		}

	private:
		ForwardList(const ForwardList& other) {}
		ForwardList(ForwardList&& other) {}
		ForwardList& operator=(const ForwardList& other) {}
		ForwardList& operator=(ForwardList&& other) {}

		ForwardList& operator==(const ForwardList& other) {}
		ForwardList& operator!=(const ForwardList& other) {}
		ForwardList& operator<(const ForwardList& other) {}
		ForwardList& operator>(const ForwardList& other) {}
		ForwardList& operator<=(const ForwardList& other) {}
		ForwardList& operator>=(const ForwardList& other) {}

	public:
		Node* m_Front;
		Node* m_Rear;
	};
}