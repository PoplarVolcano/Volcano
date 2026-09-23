#include "BlockAllocator.h"

namespace volstl
{

	std::once_flag BlockAllocator::init_flag;
	std::shared_ptr<BlockAllocator> BlockAllocator::instance;

	const uint32_t ChunkSize = 16 * 1024;     // 块堆尺寸B
	const uint32_t MaxBlockSize = 640;        // 最大块尺寸B
	const uint8_t ChunkArrayIncrement = 128;  // 块堆数组增量，128块

	// 支持的对象大小，单位B。分配时向上取整
	const uint32_t BlockSizes[BlockSizeCount] =
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

	// 将块映射到MaxBlockSize中的合适插槽。
	// values[0-16]    = 0;
	// values[17-32]   = 1;
	// values[33-64]   = 2;
	// …
	// values[513-640] = 13;
	struct SizeMap
	{
		SizeMap()
		{
			uint32_t j = 0;
			values[0] = 0;
			for (uint32_t i = 1; i != MaxBlockSize + 1; i++)
			{
				if (i <= BlockSizes[j])
				{
					values[i] = j;
				}
				else
				{
					j++;
					values[i] = j;
				}
			}
		}
		uint32_t values[MaxBlockSize + 1];
	};

	// 一个预计算数组。SizeMapInstance.values[20] 因为20≤32，所以等于索引1（代表32B）。这实现了 “向上取整”
	const SizeMap SizeMapInstance;

	// 块堆chunk
	// blockSize：块的尺寸，blocks：块数组
	struct Chunk
	{
		uint32_t blockSize;
		Block* blocks;
	};

	// 块block
	struct Block
	{
		Block* next;
	};

	void* AllocDefault(size_t size)
	{
		return malloc(size);
	}

	void FreeDefault(void* mem)
	{
		free(mem);
	}

	// 实现此函数以使用自己的内存分配器
	// ::operator new(size);
	inline void* CustomAlloc(size_t size)
	{
		return ::operator new(size);
		//return AllocDefault(size);
	}

	// ::operator delete(mem, size);
	inline void CustomFree(void* mem, size_t size)
	{
		::operator delete(mem, size);
		//FreeDefault(mem);
	}

	BlockAllocator::BlockAllocator()
	{
		m_ChunkSpace = ChunkArrayIncrement;
		m_ChunkCount = 0;
		m_Chunks = (Chunk*)CustomAlloc(m_ChunkSpace * sizeof(Chunk));

		memset(m_Chunks, 0, m_ChunkSpace * sizeof(Chunk));
		memset(m_FreeLists, 0, sizeof(m_FreeLists));
	}

	BlockAllocator::~BlockAllocator()
	{
		for (size_t i = 0; i < m_ChunkCount; ++i)
		{
			CustomFree(m_Chunks[i].blocks, ChunkSize);
		}

		CustomFree(m_Chunks, m_ChunkSpace * sizeof(Chunk));
	}

	void* BlockAllocator::Allocate(size_t size)
	{
		if (size <= 0)
		{
			return nullptr;
		}

		if (size > MaxBlockSize)
		{
			return CustomAlloc(size);
		}

		uint32_t index = SizeMapInstance.values[size];

		// 是否有空闲块
		if (m_FreeLists[index])
		{
			Block* block = m_FreeLists[index];
			m_FreeLists[index] = block->next;
#if 0
			std::cout << "内存池分配" << BlockSizes[index] << "B\n";
#endif
			return block;
		}
		else
		{
			// 块堆数组满，扩大块堆上限
			if (m_ChunkCount == m_ChunkSpace)
			{
				Chunk* oldChunks = m_Chunks;
				m_ChunkSpace += ChunkArrayIncrement;
				m_Chunks = (Chunk*)CustomAlloc(m_ChunkSpace * sizeof(Chunk));
				memcpy(m_Chunks, oldChunks, m_ChunkCount * sizeof(Chunk));
				memset(m_Chunks + m_ChunkCount, 0, ChunkArrayIncrement * sizeof(Chunk));
				CustomFree(oldChunks, m_ChunkCount * sizeof(Chunk));
			}

			// 新块堆初始化
			Chunk* chunk = m_Chunks + m_ChunkCount;
			chunk->blocks = (Block*)CustomAlloc(ChunkSize);

			uint32_t blockSize = BlockSizes[index]; 
			chunk->blockSize = blockSize;
			uint32_t blockCount = ChunkSize / blockSize;

			// 将块堆初始化为块数组，其中每一个块保存指向下一个块地址的指针(空闲块)，当某块被使用时，指针被覆盖
			for (uint32_t i = 0; i < blockCount - 1; ++i)
			{
				Block* block = (Block*)((char*)chunk->blocks + blockSize * i);
				Block* next = (Block*)((char*)chunk->blocks + blockSize * (i + 1));
				block->next = next;
			}
			Block* lastBlock = (Block*)((char*)chunk->blocks + blockSize * (blockCount - 1));
			lastBlock->next = nullptr;

			m_FreeLists[index] = chunk->blocks->next;
			m_ChunkCount++;
#if 0
			std::cout << "内存池分配" << BlockSizes[index] << "B\n";
#endif

			return chunk->blocks;
		}
	}

	void BlockAllocator::Free(void* p, size_t size)
	{
		if (!p) // 空指针，空操作
			return;

		if (size == 0)
			return;

		if (size > MaxBlockSize)
		{
			CustomFree(p, size);
			return;
		}

		uint32_t index = SizeMapInstance.values[size];
		assert(index < BlockSizeCount);

#if defined(_DEBUG)

		uint32_t blockSize = BlockSizes[index];
		bool found = false;
		for (size_t i = 0; i < m_ChunkCount; ++i)
		{
			Chunk* chunk = m_Chunks + i;
			if (chunk->blockSize == blockSize)
			{
				if ((char*)chunk->blocks <= (char*)p && (char*)p + blockSize <= (char*)chunk->blocks + ChunkSize)
				{
					found = true;
				}
			}
			else
			{
				assert((char*)p + blockSize <= (char*)chunk->blocks || (char*)chunk->blocks + ChunkSize <= (char*)p);
			}
		}

		assert(found);

		memset(p, 0xfd, blockSize);
#endif

		// 将p地址的对象设置成指向下一空闲块的指针，即将p地址设置成空闲块
		Block* block = (Block*)p;
		block->next = m_FreeLists[index];
		m_FreeLists[index] = block;
#if 0
		std::cout << "内存池释放" << BlockSizes[index] << "B\n";
#endif
	}

	void BlockAllocator::Clear()
	{
		for (size_t i = 0; i < m_ChunkCount; ++i)
		{
			CustomFree(m_Chunks[i].blocks, ChunkSize);
		}

		m_ChunkCount = 0;
		memset(m_Chunks, 0, m_ChunkSpace * sizeof(Chunk));
		memset(m_FreeLists, 0, sizeof(m_FreeLists));
	}


}