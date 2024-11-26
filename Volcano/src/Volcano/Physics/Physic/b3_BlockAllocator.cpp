#include "volpch.h"

#include "b3_BlockAllocator.h"
#include "b3_Setting.h"

namespace Volcano {

	const int b3_ChunkSize = 16 * 1024;      // ��ߴ�
	const int b3_MaxBlockSize = 640;         // ��������
	const int b3_ChunkArrayIncrement = 128;  // ����������

	// These are the supported object sizes. Actual allocations are rounded up the next size.
	// ��Щ��֧�ֵĶ����С��ʵ�ʷ��䰴��һ����С�������롣
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
	// �⽫��������Сӳ�䵽b3_MaxBlockSize�еĺ��ʲ�ۡ�
	/*
	values = 
	{
	    0     { 0 },
	    1-16  { 0 },
		17-32 { 1 },
		32-64 { 2 },
		����
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

	// ���chunk, blockSize����ĳߴ磬blocks��������
	struct b3_Chunk
	{
		int blockSize;
		b3_Block* blocks;
	};

	// ��block
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
		memset(m_freeLists, 0, sizeof(m_freeLists));   // ��m_FreeLists��14����ָ���ʼ��Ϊ��ָ��
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

		// ����size��С��ȡ�ڴ������
		int index = b3_SizeMapInstance.values[size];

		if (m_freeLists[index])
		{
			b3_Block* block = m_freeLists[index];
			m_freeLists[index] = block->next; // ���ؿ飬����ָ����һ���飨���һ�����nextΪ��ָ�룬ִ��else�¼ӿ�ѷ����ڴ棩
			return block;
		}
		else
		{
			// ��������ˣ����������
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
			chunk->blocks = (b3_Block*)b3_Alloc(b3_ChunkSize);  // ����һ��16*1024���ڴ�ռ�
#if defined(_DEBUG)
			//memset(chunk->blocks, 0xcd, b3_ChunkSize);
#endif
			int blockSize = b3_BlockSizes[index];     // ��ߴ�
			chunk->blockSize = blockSize;
			int blockCount = b3_ChunkSize / blockSize;// ����м�����

			// ���ÿ�ѵĿ�����
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
		// ��֤�ڴ��ַ�ʹ�С�Ƿ���Ч�� Verify the memory address and size is valid.
		// ���ָ��p��ĳһ��ѵ�blocksָ�뷶Χ��
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