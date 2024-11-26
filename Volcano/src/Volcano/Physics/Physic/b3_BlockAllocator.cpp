#include "volpch.h"

#include "b3_BlockAllocator.h"
#include "b3_Setting.h"

namespace Volcano {

	const int b3_ChunkSize = 16 * 1024;      // 块尺寸
	const int b3_MaxBlockSize = 640;         // 最大块数量
	const int b3_ChunkArrayIncrement = 128;  // 块数组增量

	// These are the supported object sizes. Actual allocations are rounded up the next size.
	// 这些是支持的对象大小。实际分配按下一个大小四舍五入。
	const int b3_BlockSizes[b3_BlockSizeCount] =
	{
		16,		// 0 
		32,		// 1  16 + 16
		64,		// 2  32 + 32
		96,		// 3  64 + 32
		128,	// 4  64 + 64
		160,	// 5  128 + 32
		192,	// 6  128 + 64
		224,	// 7  128 + 96
		256,	// 8  128 + 128
		320,	// 9  256 + 64
		384,	// 10 256 + 128
		448,	// 11 256 + 192
		512,	// 12 256 + 256
		640,	// 13 512 + 128
	};

	// This maps an arbitrary allocation size to a suitable slot in b3_MaxBlockSize.
	// 这将任意分配大小映射到b3_MaxBlockSize中的合适插槽。
	/*
	values = 
	{
	    0     { 0 },
	    1-16  { 0 },
		17-32 { 1 },
		32-64 { 2 },
		……
		513-640 { 13 }
	}
	*/
	struct b3_SizeMap
	{
		b3_SizeMap()
		{
			int j = 0;
			values[0] = 0;
			for (int i = 1; i <= b3_MaxBlockSize; ++i)
			{
				if (i <= b3_BlockSizes[j])
				{
					values[i] = (unsigned char)j;
				}
				else
				{
					++j;
					values[i] = (unsigned char)j;
				}
			}
		}

		unsigned char values[b3_MaxBlockSize + 1];
	};

	const b3_SizeMap b3_SizeMapInstance;

	// 块堆chunk, blockSize：块的尺寸，blocks：块数组
	struct b3_Chunk
	{
		int blockSize;
		b3_Block* blocks;
	};

	// 块block
	struct b3_Block
	{
		b3_Block* next;
	};

	b3_BlockAllocator::b3_BlockAllocator()
	{
		m_chunkSpace = b3_ChunkArrayIncrement;
		m_chunkCount = 0;
		m_chunks = (b3_Chunk*)b3_Alloc(m_chunkSpace * sizeof(b3_Chunk));

		memset(m_chunks, 0, m_chunkSpace * sizeof(b3_Chunk));
		memset(m_freeLists, 0, sizeof(m_freeLists));   // 将m_FreeLists的14个块指针初始化为空指针
	}

	b3_BlockAllocator::~b3_BlockAllocator()
	{
		for (int i = 0; i < m_chunkCount; ++i)
		{
			b3_Free(m_chunks[i].blocks);
		}

		b3_Free(m_chunks);
	}

	void* b3_BlockAllocator::Allocate(int size)
	{
		if (size <= 0)
		{
			return nullptr;
		}

		if (size > b3_MaxBlockSize)
		{
			return b3_Alloc(size);
		}

		// 按照size大小获取内存块索引
		int index = b3_SizeMapInstance.values[size];

		if (m_freeLists[index])
		{
			b3_Block* block = m_freeLists[index];
			m_freeLists[index] = block->next; // 返回块，索引指向下一个块（最后一个块的next为空指针，执行else新加块堆分配内存）
			return block;
		}
		else
		{
			// 如果块满了，扩大块上限
			if (m_chunkCount == m_chunkSpace)
			{
				b3_Chunk* oldChunks = m_chunks;
				m_chunkSpace += b3_ChunkArrayIncrement;
				m_chunks = (b3_Chunk*)b3_Alloc(m_chunkSpace * sizeof(b3_Chunk));
				memcpy(m_chunks, oldChunks, m_chunkCount * sizeof(b3_Chunk));
				memset(m_chunks + m_chunkCount, 0, b3_ChunkArrayIncrement * sizeof(b3_Chunk));
				b3_Free(oldChunks);
			}

			b3_Chunk* chunk = m_chunks + m_chunkCount;
			chunk->blocks = (b3_Block*)b3_Alloc(b3_ChunkSize);  // 分配一个16*1024的内存空间
#if defined(_DEBUG)
			//memset(chunk->blocks, 0xcd, b3_ChunkSize);
#endif
			int blockSize = b3_BlockSizes[index];     // 块尺寸
			chunk->blockSize = blockSize;
			int blockCount = b3_ChunkSize / blockSize;// 块堆有几个块

			// 设置块堆的块数组
			for (int i = 0; i < blockCount - 1; ++i)
			{
				b3_Block* block = (b3_Block*)((char*)chunk->blocks + blockSize * i);
				b3_Block* next = (b3_Block*)((char*)chunk->blocks + blockSize * (i + 1));
				block->next = next;
			}
			b3_Block* last = (b3_Block*)((char*)chunk->blocks + blockSize * (blockCount - 1));
			last->next = nullptr;

			m_freeLists[index] = chunk->blocks->next;
			++m_chunkCount;

			return chunk->blocks;
		}
	}

	void b3_BlockAllocator::Free(void* p, int size)
	{
		if (size == 0)
		{
			return;
		}

		assert(0 < size);

		if (size > b3_MaxBlockSize)
		{
			b3_Free(p);
			return;
		}

		int index = b3_SizeMapInstance.values[size];
		assert(0 <= index && index < b3_BlockSizeCount);

#if defined(_DEBUG)
		// 验证内存地址和大小是否有效。 Verify the memory address and size is valid.
		// 检查指针p在某一块堆的blocks指针范围内
		int blockSize = b3_BlockSizes[index];
		bool found = false;
		for (int i = 0; i < m_chunkCount; ++i)
		{
			b3_Chunk* chunk = m_chunks + i;
			if (chunk->blockSize != blockSize)
			{
				assert((char*)p + blockSize <= (char*)chunk->blocks || (char*)chunk->blocks + b3_ChunkSize <= (char*)p);
			}
			else
			{
				if ((char*)chunk->blocks <= (char*)p && (char*)p + blockSize <= (char*)chunk->blocks + b3_ChunkSize)
				{
					found = true;
				}
			}
		}

		assert(found);

		memset(p, 0xfd, blockSize);
#endif

		b3_Block* block = (b3_Block*)p;
		block->next = m_freeLists[index];
		m_freeLists[index] = block;
	}

	void b3_BlockAllocator::Clear()
	{
		for (int i = 0; i < m_chunkCount; ++i)
		{
			b3_Free(m_chunks[i].blocks);
		}

		m_chunkCount = 0;
		memset(m_chunks, 0, m_chunkSpace * sizeof(b3_Chunk));
		memset(m_freeLists, 0, sizeof(m_freeLists));
	}

}