#pragma once

#include "volstl/Iterator/ArrayIterator.h"

// debug模式下，检测索引是否越界
#if _DEBUG > 0
#define ARRAY_OUT_OF_RANGE_ASSERT assert(index < N && "Array subscript out of range")
#else
#define ARRAY_OUT_OF_RANGE_ASSERT
#endif

namespace volstl
{
	// 长度固定的顺序表(数组)
	template<typename T, size_t N>
	class Array
	{
	public:
		using ElementType = T;
		using Iterator = ArrayIterator<Array<ElementType, N>>;
		using ConstIterator = ArrayConstIterator<Array<ElementType, N>>;
	public:
		Array() {}

		template<typename U>
		Array(std::initializer_list<U> initList) 
		{
			// 运行期检查
			if (initList.size() > N) 
				throw std::out_of_range("Number of initializers out of range");

			size_t index = 0;
			for (auto it = initList.begin(); it != initList.end(); it++)
			{
				new(&m_Data[index++]) ElementType(static_cast<std::remove_const_t<U>&&>(const_cast<U&>(*it)));
			}
			while (index != N)
			{
				new(&m_Data[index++]) ElementType();
			}
		}

		~Array() {}

		void Fill(const ElementType& value)
		{
			for (ElementType& item : *this)
			{
				item = value;
			}
		}

		void Swap(Array& other)
		{
			if (other.m_Data == m_Data) {
				return;
			}
			for (size_t i = 0; i < N; i++)
			{
				std::swap(m_Data[i], other.m_Data[i]); // intentional ADL
			}
		}

		ElementType& operator[](size_t index)
		{
			ARRAY_OUT_OF_RANGE_ASSERT;
			return m_Data[index];
		}

		const ElementType& operator[](size_t index) const
		{
			ARRAY_OUT_OF_RANGE_ASSERT;
			return m_Data[index];
		}

		ElementType& At(size_t index)
		{
			ARRAY_OUT_OF_RANGE_ASSERT;
			return m_Data[index];
		}

		const ElementType& At(size_t index) const
		{
			ARRAY_OUT_OF_RANGE_ASSERT;
			return m_Data[index];
		}

		ElementType& Front()
		{
			return m_Data[0];
		}

		const ElementType& Front() const
		{
			return m_Data[0];
		}

		ElementType& Back()
		{
			return m_Data[N - 1];
		}

		const ElementType& Back() const
		{
			return m_Data[N - 1];
		}

		ElementType* Data() 
		{
			return m_Data; 
		}

		const ElementType* Data() const 
		{ 
			return m_Data; 
		}

		const size_t Size() const 
		{ 
			return N;
		}

		const size_t MaxSize() const
		{
			return N;
		}
		
		const bool Empty() 
		{ 
			return false;
		}

		Iterator begin()
		{
			return Iterator(m_Data);
		}

		ConstIterator begin() const
		{
			return ConstIterator(m_Data);
		}

		Iterator end()
		{
			return Iterator(m_Data + N);
		}

		ConstIterator end() const
		{
			return ConstIterator(m_Data + N);
		}

	private:
		Array(const Array& other) {}
		Array(Array&& other) {}
		Array& operator=(const Array& other) {}
		Array& operator=(Array&& other) {}
		bool operator==(Array&& other) {}
		bool operator!=(Array&& other) {}
		bool operator<(Array&& other) {}
		bool operator>(Array&& other) {}
		bool operator<=(Array&& other) {}
		bool operator>=(Array&& other) {}

	public:
		ElementType m_Data[N];
	};



}