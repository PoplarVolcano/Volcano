#pragma once

namespace Volcano {
	
	template<typename T>
	class Queue
	{
	public:

		Queue() {
			m_data = (T*)malloc(m_space * sizeof(T));
			m_head = m_data;
			m_tail = m_head;
		}

		void push(T& t)
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

		void pop()
		{
			if (m_size == 0)
				return;

			m_size--;

			if (m_head - m_data == m_space - 1)
				m_head = m_data;
			else
				m_head++;
		}

		T front()
		{
			if (m_head - m_data == m_space - 1)
				return m_data[0];
			else
				return *m_head;
		}

		T back()
		{
			if (m_tail - m_data == 0)
				return m_data[m_space - 1];
			else
				return *m_tail;
		}

		size_t size()
		{
			return m_size;
		}

		bool empty()
		{
			return m_size == 0;
		}

		void resize(size_t newSize)
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

		T* m_data;
		size_t m_size = 0;
		size_t m_space = 2; // 初始2个内存空间，1个用于存放空指针head
		T* m_head;
		T* m_tail;    // head空指针，head + 1为首元素，tail为尾元素

	};
}