#pragma once

#include <list>

#include "volstl/Iterator/ListIterator.h"
#include "volstl/BlockAllocator.h"

namespace volstl
{

	template<typename T>
	struct LNode
	{
		T data;
		LNode* prior;
		LNode* next;
	};

	// 带头结点循环双链表
	template<typename T>
	class List
	{
	public:
		using ElementType = T;
		using Node = LNode<ElementType>;
		using Iterator = ListIterator<List<ElementType>>;
		using ConstIterator = ListConstIterator<List<ElementType>>;
	public:
		List() 
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node)))
		{
			m_Front->prior = m_Front;
			m_Front->next = m_Front;
		}

		~List()
		{
			Clear();
			// m_Front->~Node(); 头结点只申请内存，未执行构造，不需要执行析构
			volstl::BlockAllocator::GetInstance()->Free(m_Front, sizeof(Node));
		}

		List(size_t count, const ElementType& value)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node)))
		{
			m_Front->prior = m_Front;
			m_Front->next = m_Front;
			for (size_t i = 0; i != count; i++)
				EmplaceBack(value);
		}

		List(size_t count)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node)))
		{
			m_Front->prior = m_Front;
			m_Front->next = m_Front;
			ElementType temp;
			for (size_t i = 0; i != count; i++)
				EmplaceBack(temp);
		}

		// [First, last)
		List(ConstIterator first, ConstIterator last)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node)))
		{
			m_Front->prior = m_Front;
			m_Front->next = m_Front;
#if _DEBUG > 0
			if (first == last || first.m_Ptr == nullptr || last.m_Ptr == nullptr)
				assert("List: Invalid Iterator");

			// 检测first和last是否在同一容器中
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == first.m_Ptr)
					assert("List: Invalid Iterator");
			}
#endif
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				EmplaceBack(ptr->data);
			}
		}

		template<typename U>
		List(std::initializer_list<U> initList)
			: m_Front((Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node)))
		{
			m_Front->prior = m_Front;
			m_Front->next = m_Front;
			for (auto it = initList.begin(); it != initList.end(); it++)
			{
				EmplaceBack(static_cast<std::remove_const_t<U>&&>(const_cast<U&>(*it)));
			}
		}

		Iterator Insert(ConstIterator where, const ElementType& value)
		{
			return Emplace(where, value);
		}

		Iterator Insert(ConstIterator where, ElementType&& value)
		{
			return Emplace(where, std::forward<ElementType>(value));
		}

		// 在index结点的前面插入新结点
		Iterator Insert(size_t where, const ElementType& value)
		{
			return Insert(Iterator(GetById(where)), value);
		}

		Iterator Insert(size_t where, ElementType&& value)
		{
			return Insert(Iterator(GetById(where)), std::forward<ElementType>(value));
		}

		Iterator PushBack(const ElementType& value)
		{
			return Insert(end(), value);
		}

		Iterator PushBack(ElementType&& value)
		{
			return Insert(end(), std::forward<ElementType>(value));
		}

		Iterator PushFront(const ElementType& value)
		{
			return Insert(begin(), value);
		}

		Iterator PushFront(ElementType&& value)
		{
			return Insert(begin(), std::forward<ElementType>(value));
		}

		template<typename... Args>
		Iterator Emplace(ConstIterator where, Args&&... args)
		{
			Node* newNode = (Node*)volstl::BlockAllocator::GetInstance()->Allocate(sizeof(Node));
			new(&newNode->data) ElementType(std::forward<Args>(args)...);
			where.m_Ptr->prior->next = newNode;
			newNode->prior           = where.m_Ptr->prior;
			newNode->next            = where.m_Ptr;
			where.m_Ptr->prior       = newNode;
			return Iterator(newNode);
		}

		template<typename... Args>
		Iterator Emplace(size_t where, Args&&... args)
		{
			return Emplace(ConstIterator(GetById(where)), std::forward<Args>(args)...);
		}

		template<typename... Args>
		Iterator EmplaceBack(Args&&... args)
		{
			return Emplace(end(), std::forward<Args>(args)...);
		}

		template<typename... Args>
		Iterator EmplaceFront(Args&&... args)
		{
			return Emplace(begin(), std::forward<Args>(args)...);
		}

		// 返回删除前该结点的下一结点的迭代器
		Iterator Erase(ConstIterator where)
		{
#if _DEBUG > 0
			if (where.m_Ptr == nullptr)
				assert("List::Erase: Invalid Iterator");
			// 检测pos是否在本容器中
			for (ConstIterator it = begin(); it != where; it++)
			{
				if (it == end())
					assert("List::Erase: Invalid Iterator");
			}
#endif
			if (where.m_Ptr == m_Front)
				return begin();
			Node* node               = where.m_Ptr->next;
			where.m_Ptr->prior->next = where.m_Ptr->next;
			where.m_Ptr->next->prior = where.m_Ptr->prior;
			where.m_Ptr->~Node();
			volstl::BlockAllocator::GetInstance()->Free(where.m_Ptr, sizeof(Node));
			return Iterator(node);
		}

		// 区间删除，[first, last)
		Iterator Erase(ConstIterator first, ConstIterator last)
		{
#if _DEBUG > 0
			if (Empty() || first == last || first.m_Ptr == m_Front || first.m_Ptr == nullptr || last == nullptr)
				assert("List::Erase: Invalid Iterator");

			// 检验last是否在first之后
			for (ConstIterator it = first ; it != last; it++)
			{
				if (it == end())
					assert("List::Erase: Invalid Iterator");
			}
#endif
			for (; first != last;)
				first = Erase(first);

			return Iterator(first.m_Ptr);
		}

		void PopBack()
		{
			Erase(--end());
		}

		void PopFront()
		{
			Erase(begin());
		}

		void Assign(size_t count, const ElementType& value)
		{
			Clear();
			for (size_t i = 0; i != count; i++)
				EmplaceBack(value);
		}

		// 销毁 vector 中所有元素，并用迭代器范围 [first, last) 的内容填充
		void Assign(Iterator first, Iterator last)
		{
#if _DEBUG > 0
			if (first == last || first.m_Ptr == nullptr || last.m_Ptr == nullptr)
				assert("List::Assign: Invalid Iterator");

			// 检测first和last是否在同一容器中
			for (Node* ptr = first.m_Ptr; ptr != last.m_Ptr; ptr = ptr->next)
			{
				if (ptr->next == first.m_Ptr)
					assert("List::Assign: Invalid Iterator");
			}
#endif
			for (; first != last; first++)
				EmplaceBack(*first);
		}

		void Swap(List<ElementType>& other)
		{
			if (m_Front == other.m_Front)
				return;

			Node* front = other.m_Front;
			other.m_Front = m_Front;
			m_Front = front;
		}

		void Clear()
		{
			while (!Empty())
			{
				PopBack();
			}
		}

		void Resize(size_t count)
		{
			size_t size = Size();
			if (count > size)
			{
				ElementType temp;
				for (size_t i = size; i != count; i++)
					EmplaceBack(temp);
			}
			else
			{
				for (size_t i = size - count; i != 0; i--)
					Erase(--end());
			}
		}

		Iterator GetByValue(const ElementType& value)
		{
			Node* node = m_Front->next;
			while (node != m_Front)
				if (node->data == value)
					return Iterator(node);
				else
					node = node->next;
			return end();
		}

		Iterator GetById(size_t index)
		{
#if _DEBUG > 0
			assert(index < Size() && "List subscript out of range");
#endif
			Node* node = m_Front->next;
			for (size_t i = 0; i != index; i++)
				node = node->next;

			// TODO: 从链尾开始遍历

			return Iterator(node);
		}

		ElementType& Front()
		{
			return m_Front->next->data;
		}

		ElementType& back()
		{
			return m_Front->prior->data;
		}

		// 将other全部结点移动到pos
		void Splice(ConstIterator pos, List& other)
		{
#if _DEBUG > 0
			if (pos.m_Ptr == nullptr)
				assert("List::Splice: Invalid Iterator");
			// 检测pos是否在本容器中
			for (ConstIterator it = begin(); it != pos; it++)
			{
				if (it == end())
					assert("List::Splice: Invalid Iterator");
			}
#endif
			pos.m_Ptr->prior->next      = other.begin().m_Ptr;
			other.begin().m_Ptr->prior  = pos.m_Ptr->prior;
			pos.m_Ptr->prior            = (--other.end()).m_Ptr;
			(--other.end()).m_Ptr->next = pos.m_Ptr;
			other.end().m_Ptr->next     = other.end().m_Ptr;
			other.end().m_Ptr->prior    = other.end().m_Ptr;
		}

		// 将other中it指向的元素移动到pos
		void Splice(ConstIterator pos, List& other, ConstIterator otherIt)
		{
#if _DEBUG > 0
			if (pos.m_Ptr == nullptr)
				assert("List::Splice: Invalid Iterator: pos");
			if (otherIt.m_Ptr == nullptr || otherIt == other.end())
				assert("List::Splice: Invalid Iterator: otherIt");
			// 检测pos是否在本容器中
			for (ConstIterator it = begin(); it != pos; it++)
			{
				if (it == end())
					assert("List::Splice: Invalid Iterator: pos");
			}
			// 检测otherIt是否other中
			for (ConstIterator it = other.begin(); it != otherIt; it++)
			{
				if (it == other.end())
					assert("List::Splice: Invalid Iterator: otherIt");
			}
#endif
			// 在other中移除otherIt
			otherIt.m_Ptr->next->prior  = otherIt.m_Ptr->prior;
			otherIt.m_Ptr->prior->next  = otherIt.m_Ptr->next;
			// 将otherIt插入pos前
			pos.m_Ptr->prior->next      = otherIt.m_Ptr;
			otherIt.m_Ptr->prior        = pos.m_Ptr->prior;
			pos.m_Ptr->prior            = otherIt.m_Ptr;
			otherIt.m_Ptr->next         = pos.m_Ptr;
		}
		
		// 将other中[first, last]元素移动到pos
		void Splice(ConstIterator pos, List& other, ConstIterator first, ConstIterator last)
		{
#if _DEBUG > 0
			if (pos.m_Ptr == nullptr)
				assert("List::Splice: Invalid Iterator: pos");
			if (first.m_Ptr == nullptr || first == other.end())
				assert("List::Splice: Invalid Iterator: first");
			if (last.m_Ptr == nullptr || last == other.end())
				assert("List::Splice: Invalid Iterator: first");
			// 检测pos是否在本容器中
			for (ConstIterator it = begin(); it != pos; it++)
			{
				if (it == end())
					assert("List::Splice: Invalid Iterator: pos");
			}
			// 检测first是否other中
			for (ConstIterator it = other.begin(); it != first; it++)
			{
				if (it == other.end())
					assert("List::Splice: Invalid Iterator: first");
			}
			// 检测last是否other中
			for (ConstIterator it = other.begin(); it != last; it++)
			{
				if (it == other.end())
					assert("List::Splice: Invalid Iterator: last");
			}
#endif
			// 将[first, last]从other中移除
			last.m_Ptr->next->prior     = first.m_Ptr->prior;
			first.m_Ptr->prior->next    = last.m_Ptr->next;
			// 将[first, last]插入pos前
			pos.m_Ptr->prior->next      = first.m_Ptr;
			first.m_Ptr->prior          = pos.m_Ptr->prior;
			pos.m_Ptr->prior            = last.m_Ptr;
			last.m_Ptr->next            = pos.m_Ptr;
		}

		// 合并，将已排序的other合并到已排序的list
		void Merge(List& other)
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
			for(Iterator it = begin(); it != end();)
			{
				if (*it == value)
					it = Erase(it);
				else
					it++;
			}
		}

		const size_t Size() const
		{
			size_t size = 0;
			for (auto& node : *this)
				size++;
			return size;
		}

		const bool Empty()
		{
			return m_Front->next == m_Front;
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
			return Iterator(m_Front);
		}
		ConstIterator end() const
		{
			return ConstIterator(m_Front);
		}

	private:
		List(const List& other) {}
		List(List&& other) {}
		List& operator=(const List& other) {}
		List& operator=(List&& other) {}

		List& operator==(const List& other) {}
		List& operator!=(const List& other) {}
		List& operator<(const List& other) {}
		List& operator>(const List& other) {}
		List& operator<=(const List& other) {}
		List& operator>=(const List& other) {}

	public:
		Node* m_Front;
	};
}