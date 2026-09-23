#pragma once

#include "volstl/Vector.h"

namespace volstl
{
	template<typename T>
	class DequeConstIterator
	{
	public:
		using DataStructure = T;
		using ElementType = typename DataStructure::ElementType;
	public:
		DequeConstIterator() 
			: m_ChunksPtr(nullptr), m_FrontChunk(0), m_FrontIndex(0), 
			m_BackChunk(0), m_BackIndex(0), m_Chunk(0), m_Index(0)
		{}

		DequeConstIterator(
			const Vector<std::unique_ptr<ElementType[]>>* ptr,
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

		~DequeConstIterator() {}

		// 前缀运算符，++it
		DequeConstIterator& operator++()
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
		DequeConstIterator operator++(int)
		{
			DequeConstIterator constIterator = *this;
			++(*this);
			return constIterator;
		}

		// 前缀运算符，--it
		DequeConstIterator& operator--()
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
		DequeConstIterator operator--(int)
		{
			DequeConstIterator constIterator = *this;
			--(*this);
			return constIterator;
		}

		DequeConstIterator& operator+=(const size_t count)
		{
			if (m_Index + count > (m_BackChunk - m_Chunk) * DataStructure::ChunkSize + m_BackIndex)
				assert("DequeConstIterator:operator+=(), invelid count.");
			m_Chunk = m_Chunk + (m_Index + count) / DataStructure::ChunkSize;
			m_Index = (m_Index + count) % DataStructure::ChunkSize;

			return *this;
		}

		DequeConstIterator operator+(const size_t count) const
		{
			DequeConstIterator constIterator = *this;
			constIterator += count;
			return constIterator;
		}

		DequeConstIterator& operator-=(const size_t count)
		{
			if((m_Chunk - m_FrontChunk) * DataStructure::ChunkSize + m_Index - m_FrontIndex < count)
				assert("DequeConstIterator:operator+=(), invelid count.");
			m_Chunk = (m_Chunk * DataStructure::ChunkSize + m_Index - count) / DataStructure::ChunkSize;
			m_Index = (m_Chunk * DataStructure::ChunkSize + m_Index - count) % DataStructure::ChunkSize;
			return *this;
		}

		DequeConstIterator operator-(const size_t count) const
		{
			DequeConstIterator constIterator = *this;
			constIterator -= count;
			return constIterator;
		}

		int operator-(const DequeConstIterator& other) const
		{
			return m_Chunk * DataStructure::ChunkSize + m_Index - other.m_Chunk * DataStructure::ChunkSize - other.m_Index;
		}

		const ElementType* operator->() const
		{
			return &(*m_ChunksPtr)[m_Chunk][m_Index];
		}

		const ElementType& operator*() const
		{
			return (*m_ChunksPtr)[m_Chunk][m_Index];
		}


		bool operator==(const DequeConstIterator& other) const
		{
			return m_ChunksPtr == other.m_ChunksPtr && m_Chunk == other.m_Chunk && m_Index == other.m_Index;
		}

		bool operator!=(const DequeConstIterator& other) const
		{
			return !(*this == other);
		}

		bool operator<(const DequeConstIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeConstIterator::operator<(), invelid Iterator");

			if (m_Chunk < other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index < other.m_Index)
				return true;
			else
				return false;
		}

		bool operator>(const DequeConstIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeConstIterator::operator>(), invelid Iterator");

			if (m_Chunk > other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index > other.m_Index)
				return true;
			else
				return false;
		}

		bool operator<=(const DequeConstIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeConstIterator::operator<(), invelid Iterator");

			if (m_Chunk < other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index <= other.m_Index)
				return true;
			else
				return false;
		}

		bool operator>=(const DequeConstIterator& other) const
		{
			if (m_ChunksPtr != other.m_ChunksPtr)
				assert("DequeConstIterator::operator>(), invelid Iterator");

			if (m_Chunk > other.m_Chunk)
				return true;
			else if (m_Chunk == other.m_Chunk && m_Index >= other.m_Index)
				return true;
			else
				return false;
		}


	public:
		const Vector<std::unique_ptr<ElementType[]>>* m_ChunksPtr;
		const size_t m_FrontChunk;
		const size_t m_FrontIndex;
		const size_t m_BackChunk;
		const size_t m_BackIndex;
		size_t m_Chunk;
		size_t m_Index;
	};
}