#pragma once

#include "volstl/Iterator/DequeConstIterator.h"

namespace volstl
{
	template<typename T>
	class DequeIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
	public:
		DequeIterator()
			: m_ChunksPtr(nullptr), m_FrontChunk(0), m_FrontIndex(0), 
			m_BackChunk(0), m_BackIndex(0), m_Chunk(0), m_Index(0)
		{}
		DequeIterator(
			Vector<std::unique_ptr<ElementType[]>>* ptr,
			const size_t frontChunk,
			const size_t frontIndex,
			const size_t backChunk,
			const size_t backIndex,
			size_t chunk,
			size_t index
		)
			: m_ChunksPtr(ptr), m_FrontChunk(frontChunk), m_FrontIndex(frontIndex),
			m_BackChunk(backChunk), m_BackIndex(backIndex), m_Chunk(chunk), m_Index(index)
		{}

		~DequeIterator() {}

		operator DequeConstIterator<DataStructure>() const
		{
			return DequeConstIterator<DataStructure>(
				m_ChunksPtr, m_FrontChunk, m_FrontIndex, 
				m_BackChunk, m_BackIndex, m_Chunk, m_Index
			);
		}

		// 前缀运算符，++it
		DequeIterator& operator++()
		{
			if (m_Chunk == m_BackChunk)
			{
				if (m_Index == m_BackIndex)
					assert("DequeIterator::operator++(), out of range");
				else
					m_Index++;
			}
			else
			{
				if (m_Index == DataStructure::ChunkSize - 1)
				{
					m_Chunk++;
					m_Index = 0;
				}
				else
				{
					m_Index++;
				}
			}
			return *this;
		}

		// 后缀运算符，it++
		DequeIterator operator++(int)
		{
			DequeIterator iterator = *this;
			++(*this);
			return iterator;
		}

		// 前缀运算符，--it
		DequeIterator& operator--()
		{
			if (m_Chunk == m_FrontChunk)
			{
				if (m_Index == m_FrontIndex)
					assert("DequeIterator::operator--(), out of range");
				m_Index--;
			}
			else
			{
				if (m_Index == 0)
				{
					m_Chunk--;
					m_Index = DataStructure::ChunkSize - 1;
				}
				else
				{
					m_Index--;
				}
			}
			return *this;
		}

		// 后缀运算符，it--
		DequeIterator operator--(int)
		{
			DequeIterator iterator = *this;
			--(*this);
			return iterator;
		}

		DequeIterator& operator+=(const size_t count)
		{
			if (m_Index + count > (m_BackChunk - m_Chunk) * DataStructure::ChunkSize + m_BackIndex)
				assert("DequeIterator:operator+=(), invelid count.");
			m_Chunk = m_Chunk + (m_Index + count) / DataStructure::ChunkSize;
			m_Index = (m_Index + count) % DataStructure::ChunkSize;

			return *this;
		}

		DequeIterator operator+(const size_t count) const
		{
			DequeIterator iterator = *this;
			iterator += count;
			return iterator;
		}

		DequeIterator& operator-=(const size_t count)
		{
			if ((m_Chunk - m_FrontChunk) * DataStructure::ChunkSize + m_Index - m_FrontIndex < count)
				assert("DequeIterator:operator+=(), invelid count.");
			m_Chunk = (m_Chunk * DataStructure::ChunkSize + m_Index - count) / DataStructure::ChunkSize;
			m_Index = (m_Chunk * DataStructure::ChunkSize + m_Index - count) % DataStructure::ChunkSize;
			return *this;
		}

		DequeIterator operator-(const size_t count) const
		{
			DequeIterator iterator = *this;
			iterator -= count;
			return iterator;
		}

		int operator-(const DequeIterator& other) const
		{
			return m_Chunk * DataStructure::ChunkSize + m_Index - other.m_Chunk * DataStructure::ChunkSize - other.m_Index;
		}

		ElementType* operator->()
		{
			return &(*m_ChunksPtr)[m_Chunk][m_Index];
		}

		const ElementType* operator->() const
		{
			return &(*m_ChunksPtr)[m_Chunk][m_Index];
		}

		ElementType& operator*()
		{
			return (*m_ChunksPtr)[m_Chunk][m_Index];
		}

		const ElementType& operator*() const
		{
			return (*m_ChunksPtr)[m_Chunk][m_Index];
		}

		bool operator==(const DequeIterator& other) const
		{
			return m_ChunksPtr == other.m_ChunksPtr && m_Chunk == other.m_Chunk && m_Index == other.m_Index;
		}

		bool operator!=(const DequeIterator& other) const
		{
			return !(*this == other);
		}

		bool operator<(const DequeIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeIterator::operator<(), invelid Iterator");

			if (m_Chunk < other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index < other.m_Index)
				return true;
			else
				return false;
		}

		bool operator>(const DequeIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeIterator::operator>(), invelid Iterator");

			if (m_Chunk > other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index > other.m_Index)
				return true;
			else
				return false;
		}

		bool operator<=(const DequeIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeIterator::operator<(), invelid Iterator");

			if (m_Chunk < other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index <= other.m_Index)
				return true;
			else
				return false;
		}

		bool operator>=(const DequeIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeIterator::operator>(), invelid Iterator");

			if (m_Chunk > other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index >= other.m_Index)
				return true;
			else
				return false;
		}


	public:
		Vector<std::unique_ptr<ElementType[]>>* m_ChunksPtr;
		const size_t m_FrontChunk;
		const size_t m_FrontIndex;
		const size_t m_BackChunk;
		const size_t m_BackIndex;
		size_t m_Chunk;
		size_t m_Index;
	};
}