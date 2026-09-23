#pragma once

namespace Volcano
{
	/*
	存储结构：使用原生数组（T* m_data）作为环形缓冲区，通过 m_head（队首指针）和 m_tail（队尾指针）维护逻辑循环
	容量管理：使用 m_size 记录当前元素个数，m_space 记录总容量（初始为 2，但实际可用空间为 m_space - 1）
	扩容策略：当 (m_size + 1) == m_space 时触发扩容，将容量翻倍，并复制旧数据

	初始化时 m_head = m_data; m_tail = m_head; m_size = 0;，即空队列


	*/
	template<typename T>
	class Queue
	{
	public:
		Queue() {
			m_data = (T*)malloc(m_space * sizeof(T));
			m_head = m_data;
			m_tail = m_head;
		}

		void Push(T& t)
		{
			if ((m_size + 1) == m_space)
			{
				size_t indexHead = m_head - m_data, indexTail = m_tail - m_data;
				T* oldData = m_data;
				m_space = m_space << 1;
				m_data = (T*)malloc(m_space * sizeof(T));
				memcpy(m_data, oldData, (m_size + 1) * sizeof(T));
				memset(m_data + (m_size + 1), 0, (m_space >> 1) * sizeof(T));
				m_head = m_data + indexHead;
				m_tail = m_data + indexTail;
				free(oldData);
			}
			m_size++;
			m_data[m_tail] = t;
			if (m_tail - m_data == m_space - 1)
				m_tail = m_data;
			else
				m_tail++;
		}

		void Pop()
		{
			if (m_size == 0)
				return;

			m_size--;

			if (m_head - m_data == m_space - 1)
				m_head = m_data;
			else
				m_head++;
		}

		T Front()
		{
			if (m_head - m_data == m_space - 1)
				return m_data[0];
			else
				return *m_head;
		}

		T Back()
		{
			if (m_tail - m_data == 0)
				return m_data[m_space - 1];
			else
				return *m_tail;
		}

		size_t Size()
		{
			return m_size;
		}

		bool Empty()
		{
			return m_size == 0;
		}

		void Resize(size_t newSize)
		{
			m_data = (T*)malloc(newSize * sizeof(T));
			m_space = newSize;
			m_head = m_data;
			m_tail = m_head;
			m_size = 0;
		}

		class Iterator
		{
		public:
			Iterator(T* node) : cur(node) {}
			// 指针运算符
			T& operator*() { return *cur; }

			// it++
			Iterator& operator++(int) {
				if (cur - m_data == m_space - 1)
					cur = m_data;
				else
					cur++;
				return *this;
			}

			bool operator!=(const Iterator& other) const { return cur != other.cur; }
			bool operator==(const Iterator& other) const { return cur == other.cur; }

		private:
			T* cur;
		};

		// 分别定义begin()、end()方法
		Iterator begin() const {
			if (size == 0)
				return Iterator(m_head);

			if (m_head - m_data == m_space - 1)
				return Iterator(m_data);
			else
				return Iterator(m_head + 1);
		}
		Iterator end() const { return Iterator(m_tail); }
	private:

		T* m_data;          // 动态分配的原始内存
		size_t m_size = 0;  // 当前元素个数
		size_t m_space = 2; // 总容量（1个用于存放空指针head，所以实际最大元素为 m_space-1），初始2个内存空间，
		T* m_head;          // 指向队首空指针
		T* m_tail;          // 指向队尾的下一个写入位置（即“尾后指针”）

	};
}