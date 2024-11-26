#pragma once

namespace Volcano {

	const int b3_BlockSizeCount = 14;

	struct b3_Chunk;
	struct b3_Block;

	// This is a small object allocator used for allocating small objects that persist for more than one time step.
	// ����һ��С�Ķ�������������ڷ������ʱ�䳬��һ��ʱ�䲽��С�Ķ���
	class b3_BlockAllocator
	{
	public:
		b3_BlockAllocator();
		~b3_BlockAllocator();

		// Allocate memory. This will use b3_Alloc if the size is larger than b3_MaxBlockSize.
		// ����b3_SizeMap�����ڴ档��size=17~32������32�ߴ���ڴ棬size=33~64������64�ߴ���ڴ档�����С����b3_MaxBlockSize����ʹ��b3_Alloc��
		void* Allocate(int size);

		/// Free memory. This will use b3_Free if the size is larger than b3_MaxBlockSize.
		void Free(void* p, int size);

		void Clear();

	private:

		b3_Chunk* m_chunks;  // ������
		int m_chunkCount;    // ������
		int m_chunkSpace;    // ��ռ�ö����ڴ�ռ�

		b3_Block* m_freeLists[b3_BlockSizeCount];
	};
}