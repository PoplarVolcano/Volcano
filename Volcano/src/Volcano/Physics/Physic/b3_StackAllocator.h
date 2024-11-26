#pragma once

namespace Volcano {

	const int b3_StackSize = 100 * 1024; // 栈内存大小100k
	const int b3_MaxStackEntries = 32;   // 栈深32

	struct b3_StackEntry
	{
		char* data;
		int size;
		bool usedMalloc;
	};
	
	/*
	This is a stack allocator used for fast per step allocations. 
    You must nest allocate/free pairs. The code will assert if you try to interleave multiple allocate/free pairs. 

	这是一个用于快速按步骤分配的堆栈分配器。
	您必须嵌套分配 / 自由配对。如果你试图交织多个分配 / 空闲对，代码将断言。﻿
	*/
	class b3_StackAllocator
	{
	public:
		b3_StackAllocator();
		~b3_StackAllocator();

		// Allocate memory. This will use b3_Alloc if the size is larger than b3_MaxBlockSize.
		// 分配内存。如果大小大于b3_MaxBlockSize，则将使用b3_Alloc。
		void* Allocate(int size);

		/// Free memory. This will use b3_Free if the size is larger than b3_MaxBlockSize.
		void Free(void* p);

		int GetMaxAllocation() const;

	private:

		char m_data[b3_StackSize]; // 栈数据
		int m_index;               // 栈索引

		int m_allocation;          // 分配内存后的内存位置，栈中有3个size为16的entry时，m_Allocation为48
		int m_maxAllocation;       // 最大内存位置，m_Allocation增加后赋值max(m_MaxAllocation, m_Allocation)

		b3_StackEntry m_entries[b3_MaxStackEntries];
		int m_entryCount;
	};
}