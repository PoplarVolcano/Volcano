#include "volpch.h"

#include "b3_StackAllocator.h"
#include "b3_Setting.h"

namespace Volcano {

	b3_StackAllocator::b3_StackAllocator()
	{
		m_index = 0;
		m_allocation = 0;
		m_maxAllocation = 0;
		m_entryCount = 0;
	}
	b3_StackAllocator::~b3_StackAllocator()
	{
		assert(m_index == 0);
		assert(m_entryCount == 0);
	}

	void* b3_StackAllocator::Allocate(int size)
	{
		assert(m_entryCount < b3_MaxStackEntries);

		b3_StackEntry* entry = m_entries + m_entryCount;
		entry->size = size;
		if (m_index + size > b3_StackSize)
		{
			entry->data = (char*)b3_Alloc(size);
			entry->usedMalloc = true;
		}
		else
		{
			entry->data = m_data + m_index;
			entry->usedMalloc = false;
			m_index += size;
		}

		m_allocation += size;
		m_maxAllocation = max(m_maxAllocation, m_allocation);
		++m_entryCount;

		return entry->data;
	}

	void b3_StackAllocator::Free(void* p)
	{
		assert(m_entryCount > 0);  // 栈空时报错
		b3_StackEntry* entry = m_entries + m_entryCount - 1; // 最后一个entry
		assert(p == entry->data);  // p不是栈顶元素时报错
		if (entry->usedMalloc)
		{
			b3_Free(p);
		}
		else
		{
			m_index -= entry->size;
		}
		m_allocation -= entry->size;
		--m_entryCount;

		p = nullptr;
	}

	int b3_StackAllocator::GetMaxAllocation() const
	{
		return m_maxAllocation;
	}
}
