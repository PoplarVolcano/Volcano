#pragma once

#include "b3_Setting.h"

namespace Volcano {

	//����һ����������LIFO(last in first out)��ջ����ʼ����ΪN��  �����ջ��С������ʼ��������ʹ�ö������Ӷ�ջ�Ĵ�С��
	template <typename T, int N>
	class b3_GrowableStack
	{
	public:
		b3_GrowableStack()
		{
			m_stack = m_array;
			m_count = 0;
			m_capacity = N;
		}

		~b3_GrowableStack()
		{
			if (m_stack != m_array)
			{
				b3_Free(m_stack);
				m_stack = nullptr;
			}
		}

		void Push(const T& element)
		{
			if (m_count == m_capacity)
			{
				T* old = m_stack;
				m_capacity *= 2;
				m_stack = (T*)b3_Alloc(m_capacity * sizeof(T));
				memcpy(m_stack, old, m_count * sizeof(T));
				if (old != m_array)
				{
					b3_Free(old);
				}
			}

			m_stack[m_count] = element;
			++m_count;
		}

		T Pop()
		{
			assert(m_count > 0);
			--m_count;
			return m_stack[m_count];
		}

		int GetCount()
		{
			return m_count;
		}

	private:
		T* m_stack;
		T m_array[N];
		int m_count;
		int m_capacity;
	};

}