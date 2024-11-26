#include "volpch.h"
#include "b3_Setting.h"

namespace Volcano {

	// Memory Allocation

	// 内存分配器。修改这些以使用您自己的分配器。
	void* b3_Alloc_Default(int size)
	{
		return malloc(size);
	}

	void b3_Free_Default(void* mem)
	{
		free(mem);
	}
}