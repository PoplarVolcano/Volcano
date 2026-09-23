#pragma once

#include <deque>
#include "volstl/Iterator/DequeIterator.h"

namespace volstl
{
	// Double-Ended Queue 双端队列
	template<typename T>
	class Deque
	{
	public:
		using ElementType = T;
		using Iterator = DequeIterator<Deque<ElementType>>;
		using ConstIterator = DequeConstIterator<Deque<ElementType>>;

	public:
		Deque()	
			: m_FrontChunk(0), m_FrontIndex(0), m_BackChunk(0), m_BackIndex(0), m_Size(0)
		{
			m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));
		}

		~Deque() {}

		// 创建包含 count 个元素的 deque，每个元素都被初始化为 value
		Deque(size_t count, const ElementType& value)
			: m_FrontChunk(0), m_FrontIndex(0), m_BackChunk(0), m_BackIndex(0), m_Size(0)
		{
			m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));
			for (size_t i = 0; i < count; i++)
				Emplace(end(), value);
		}

		Deque(size_t count)
			: m_FrontChunk(0), m_FrontIndex(0), m_BackChunk(0), m_BackIndex(0), m_Size(0)
		{
			m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));
			ElementType temp;
			for (size_t i = 0; i < count; i++)
				Emplace(end(), temp);

		}
		
		// 使用迭代器范围 [first, last) 内的元素来构造 deque
		// int arr[] = {1, 2, 3, 4, 5}; std::deque<int> d4(arr, arr + 5);
		Deque(ConstIterator first, ConstIterator last)
			: m_FrontChunk(0), m_FrontIndex(0), m_BackChunk(0), m_BackIndex(0), m_Size(0)
		{
#if _DEBUG > 0
			if (first.m_ChunksPtr == &m_Chunks)
				assert("Deque(ConstIterator first, ConstIterator last); 迭代器错误");
#endif
			m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));
			for (; first != last; first++)
				EmplaceBack(*first);
		}

		template<typename U>
		Deque(std::initializer_list<U> initList)
			: m_FrontChunk(0), m_FrontIndex(0), m_BackChunk(0), m_BackIndex(0), m_Size(0)
		{
			m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));

			for (auto it = initList.begin(); it != initList.end(); it++)
			{
				EmplaceBack(static_cast<std::remove_const_t<U>&&>(const_cast<U&>(*it)));
			}
		}

		void Assign()
		{

		}

		Iterator Insert(ConstIterator pos, const ElementType& value)
		{
			return Emplace(pos, value);
		}

		Iterator Insert(ConstIterator pos, ElementType&& value)
		{
			return Emplace(pos, std::forward<ElementType>(value));
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

		// 在pos前插入元素
		template<typename... Args>
		Iterator Emplace(ConstIterator pos, Args&&... args)
		{
			if ((pos.m_Chunk - m_FrontChunk) * ChunkSize + pos.m_Index - m_FrontIndex >= m_Size / 2)
			{
				if (m_BackIndex == ChunkSize - 1)
				{
					if (m_BackChunk == m_Chunks.Size() - 1)
						m_Chunks.PushBack(std::make_unique<ElementType[]>(ChunkSize));
					m_BackChunk++;
					m_BackIndex = 0;
				}
				else
				{
					m_BackIndex++;
				}
				m_Size++;

				Iterator tempPos(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, pos.m_Chunk, pos.m_Index);
				//从tempPos到end() - 1所有元素后移1位，在tempPos插入元素
				for (Iterator it = --end(); it != tempPos; it--)
				{
					new(&(*it)) ElementType(std::move(*(it - 1)));
					(*(it - 1)).~ElementType();
				}
				new(&(*tempPos)) ElementType(std::forward<Args>(args)...);

				return tempPos;
			}
			else
			{
				bool chunkChange = false;
				if (m_FrontIndex == 0)
				{
					if (m_FrontChunk == 0)
					{
						m_Chunks.Insert(m_Chunks.begin(), std::make_unique<ElementType[]>(ChunkSize));
						m_BackChunk++;
						chunkChange = true;
					}
					else
						m_FrontChunk--;
					m_FrontIndex = ChunkSize - 1;
				}
				else
				{
					m_FrontIndex--;
				}
				m_Size++;

				Iterator tempPos(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, pos.m_Chunk + chunkChange, pos.m_Index);
				//tempPos不动，从begin()到tempPos - 1所有元素往前挪1位，在tempPos - 1插入元素
				for (Iterator it = begin(); it != --tempPos; it++)
				{
					new(&(*it)) ElementType(std::move(*(it + 1)));
					(*(it + 1)).~ElementType();
				}
				new(&(*tempPos)) ElementType(std::forward<Args>(args)...);

				return tempPos;

			}
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

		Iterator PopBack()
		{
			if (Empty())
			{
				throw std::out_of_range("Deque is empty");
			}

			return Erase(--end());
		}

		Iterator PopFront()
		{
			if (Empty()) 
			{
				throw std::out_of_range("Deque is empty");
			}

			return Erase(begin());
		}

		Iterator Erase(ConstIterator pos)
		{
			if ((pos.m_Chunk - m_FrontChunk) * ChunkSize + pos.m_Index - m_FrontIndex >= m_Size / 2)
			{
				Iterator tempPos(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, pos.m_Chunk, pos.m_Index);
				// 释放tempPos元素，从tempPos + 1开始到end() - 1所有元素前移1位
				(*tempPos).~ElementType();
				for (Iterator it = tempPos; it != --end(); it++)
				{
					new(&(*it)) ElementType(std::move(*(it + 1)));
					(*(it + 1)).~ElementType();
				}

				if (m_BackIndex == 0)
				{
					m_BackChunk--;
					m_BackIndex = ChunkSize - 1;
				}
				else
				{
					m_BackIndex--;
				}
			}
			else
			{
				Iterator tempPos(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, pos.m_Chunk, pos.m_Index);
				// 释放tempPos元素，从tempPos - 1开始到begin()所有元素后移1位
				(*tempPos).~ElementType();
				for (Iterator it = tempPos; it != begin(); it--)
				{
					new(&(*it)) ElementType(std::move(*(it - 1)));
					(*(it - 1)).~ElementType();
				}

				if (m_FrontIndex == ChunkSize - 1)
				{
					m_FrontChunk++;
					m_FrontIndex = 0;
				}
				else
				{
					m_FrontIndex++;
				}
			}
			m_Size--;
			return Iterator(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, pos.m_Chunk, pos.m_Index);

		}

		Iterator Erase(ConstIterator first, ConstIterator last)
		{

		}

		void Swap(Deque& other)
		{

		}

		void Clear()
		{
			while (m_Size)
			{
				Erase(--end());
			}
		}

		bool Empty()
		{
			return m_Size == 0;
		}

		size_t Size()
		{
			return m_Size;
		}

		void Resize(size_t count)
		{

		}

		ElementType& Front()
		{
			if (Empty())
			{
				throw std::out_of_range("Deque is empty");
			}
			return m_Chunks[m_FrontChunk][m_FrontIndex];
		}

		const ElementType& Front() const
		{
			if (Empty())
			{
				throw std::out_of_range("Deque is empty");
			}
			return m_Chunks[m_FrontChunk][m_FrontIndex];
		}

		ElementType& Back()
		{
			if (Empty())
			{
				throw std::out_of_range("Deque is empty");
			}
			return m_Chunks[m_BackChunk][m_BackIndex];
		}

		const ElementType& Back() const
		{
			if (Empty())
			{
				throw std::out_of_range("Deque is empty");
			}
			return m_Chunks[m_BackChunk][m_BackIndex];
		}

		ElementType& operator[](size_t pos)
		{
			if (pos >= m_Size) {
				throw std::out_of_range("Pos out of range");
			}
			
			size_t chunkOffset = m_FrontIndex + pos;
			size_t chunkIndex = m_FrontChunk + chunkOffset / ChunkSize;
			size_t elemIndex = chunkOffset % ChunkSize;

			return m_Chunks[chunkIndex][elemIndex];
		}

		const ElementType& operator[](size_t pos) const
		{
			if (pos >= m_Size) {
				throw std::out_of_range("Pos out of range");
			}

			size_t chunkOffset = m_FrontIndex + pos;
			size_t chunkIndex = m_FrontChunk + chunkOffset / ChunkSize;
			size_t elemIndex = chunkOffset % ChunkSize;

			return m_Chunks[chunkIndex][elemIndex];
		}

		ElementType& At(size_t pos)
		{
			if (pos >= m_Size) {
				throw std::out_of_range("Pos out of range");
			}

			size_t chunkOffset = m_FrontIndex + pos;
			size_t chunkIndex = m_FrontChunk + chunkOffset / ChunkSize;
			size_t elemIndex = chunkOffset % ChunkSize;

			return m_Chunks[chunkIndex][elemIndex];
		}

		Iterator begin()
		{
			return Iterator(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, m_FrontChunk, m_FrontIndex);
		}

		ConstIterator begin() const
		{
			return ConstIterator(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, m_FrontChunk, m_FrontIndex);
		}

		Iterator end()
		{
			return Iterator(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, m_BackChunk, m_BackIndex);
		}

		ConstIterator end() const
		{
			return ConstIterator(&m_Chunks, m_FrontChunk, m_FrontIndex, m_BackChunk, m_BackIndex, m_BackChunk, m_BackIndex);
		}

	private:
		Deque(const Deque& other) {}
		Deque(Deque&& other) {}
		Deque& operator=(const Deque& other) {}
		Deque& operator=(Deque&& other) {}

		bool operator==(const Deque& other) const {}
		bool operator!=(const Deque& other) const {}
		bool operator<(const Deque& other) const {}
		bool operator>(const Deque& other) const {}
		bool operator<=(const Deque& other) const {}
		bool operator>=(const Deque& other) const {}

	public:
		static const size_t ElementSize = sizeof(ElementType);
		static const size_t ChunkSize = 4;
		/*
		static const size_t ChunkSize = ElementSize <= 1 ? 16
			                          : ElementSize <= 2 ? 8
			                          : ElementSize <= 4 ? 4
			                          : ElementSize <= 8 ? 2
			                          : 1; // 每个块有几个元素
									  */
	private:
		volstl::Vector<std::unique_ptr<ElementType[]>> m_Chunks;
		size_t m_FrontChunk;
		size_t m_FrontIndex;
		size_t m_BackChunk;
		size_t m_BackIndex; // end()索引
		size_t m_Size;
	};
}