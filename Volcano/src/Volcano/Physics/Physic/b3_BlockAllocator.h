#pragma once

namespace Volcano {

	const int b3_BlockSizeCount = 14;

	struct b3_Chunk;
	struct b3_Block;

	// This is a small object allocator used for allocating small objects that persist for more than one time step.
	// 这是一个小的对象分配器，用于分配持续时间超过一个时间步的小的对象。
	class b3_BlockAllocator
	{
	public:
		b3_BlockAllocator();
		~b3_BlockAllocator();

		// Allocate memory. This will use b3_Alloc if the size is larger than b3_MaxBlockSize.
		// 根据b3_SizeMap分配内存。如size=17~32，分配32尺寸的内存，size=33~64，分配64尺寸的内存。如果大小大于b3_MaxBlockSize，则将使用b3_Alloc。
		void* Allocate(int size);

		/// Free memory. This will use b3_Free if the size is larger than b3_MaxBlockSize.
		void Free(void* p, int size);

		void Clear();

	private:

		b3_Chunk* m_chunks;  // 块数组
		int m_chunkCount;    // 块数量
		int m_chunkSpace;    // 块占用多少内存空间

		b3_Block* m_freeLists[b3_BlockSizeCount];
	};
}