#include "volpch.h"
#include "b3_Setting.h"

namespace Volcano {

	// Memory Allocation

	// �ڴ���������޸���Щ��ʹ�����Լ��ķ�������
	void* b3_Alloc_Default(int size)
	{
		return malloc(size);
	}

	void b3_Free_Default(void* mem)
	{
		free(mem);
	}
}