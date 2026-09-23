#pragma once

#include "volstl/Iterator/ArrayIterator.h"
#include "volstl/BlockAllocator.h"

namespace volstl
{
	// 长度可变的顺序表
	template<typename T>
	class Vector
	{
	public:
		using ElementType = T;
		using Iterator = ArrayIterator<Vector<ElementType>>;
		using ConstIterator = ArrayConstIterator<Vector<ElementType>>;
	public:
		Vector()
		{
			Reserve(2);
		}

		~Vector()
		{
			// 调用所有元素的析构函数
			Clear();
			// 将分配的内存释放且不调用析构
			volstl::BlockAllocator::GetInstance()->Free(m_Data, m_Capacity * sizeof(ElementType));
		}

		Vector(size_t count, const ElementType& value)
		{
			Reserve(count);
			for (size_t i = 0; i < count; i++)
				EmplaceBack(value);
		}

		Vector(size_t count)
		{
			ElementType value;
			Reserve(count);
			for (size_t i = 0; i < count; i++)
				EmplaceBack(value);
		}

		//[first, last)
		Vector(ConstIterator first, ConstIterator last)
		{
#if _DEBUG > 0
			if (first.m_Ptr < begin().m_Ptr || first.m_Ptr > end().m_Ptr)
				assert("Vector(Iterator first, Iterator last)，first越界");
			if (last.m_Ptr < begin().m_Ptr || last.m_Ptr > end().m_Ptr)
				assert("Vector(Iterator first, Iterator last)，last越界");
			if (last.m_Ptr - first.m_Ptr < 0)
				assert("Vector(Iterator first, Iterator last); 迭代器错误");
#endif
			Reserve(last.m_Ptr - first.m_Ptr);
			for (; first != last; first++)
				EmplaceBack(*first.m_Ptr);
		}

		template<typename U>
		Vector(std::initializer_list<U> initList)
		{
			Reserve(initList.size());
			for (auto it = initList.begin(); it != initList.end(); it++)
			{
				/*问题记录：
				* 当 U 是 const int 时：
				* std::remove_const_t<U> 得到 int
				* std::remove_const_t<U>&& 得到 int&&
				* 但 *it 是 const int&，不能直接转换为 int&&
				* 
				* 解决方案：先移除const，然后转换为右值引用
				*/
				EmplaceBack(static_cast<std::remove_const_t<U>&&>(const_cast<U&>(*it)));
			}
		}

		Iterator Insert(ConstIterator pos, const ElementType& value)
		{
			return Emplace(pos, value);
		}
		
		Iterator Insert(ConstIterator pos, ElementType&& value)
		{
			return Emplace(pos, std::forward<ElementType>(value));
		}

		void PushBack(const ElementType& value)
		{
			Insert(ConstIterator(m_Data + m_Size), value);
		}

		void PushBack(ElementType&& value)
		{
			Insert(ConstIterator(m_Data + m_Size), std::forward<ElementType>(value));
		}

		// 把构造ElementType需要的所有参数交给vector，在vector数据块上构造实例
		// 当输入参数为左值ElementType时，EmplaceBack在m_Data[m_Size]的内存上拷贝ElementType形参
		// 当输入参数为右值ElementType时，EmplaceBack在m_Data[m_Size]的内存上移动拷贝ElementType形参，
		// std::forward保留传入参数的左值或右值的类别（通常无论注入的是左值还是右值，形参均为左值）
		template<typename... Args>
		Iterator Emplace(ConstIterator pos, Args&&... args)
		{
#if _DEBUG > 0
			if (pos.m_Ptr < begin().m_Ptr || pos.m_Ptr > end().m_Ptr)
				assert("Vector::Emplace，越界");
#endif

			// 重分配后，m_Data的地址改变，pos失效,故提取出pos的索引
			const size_t posIndex = pos.m_Ptr - begin().m_Ptr;

			if (m_Size == m_Capacity)
				Reserve(m_Capacity + (m_Capacity / 2));
			for (size_t i = m_Size; i != posIndex; i--)
			{
				// Bug记录: 对未构造的 unique_ptr 调用移动赋值运算符，这导致未定义行为
				// 解决方法：使用placement new + 手动析构
				//m_Data[i] = std::move(m_Data[i - 1]);
				new(&m_Data[i]) ElementType(std::move(m_Data[i - 1]));
				m_Data[i - 1].~ElementType();
			}

			// 在pos.m_Ptr的内存上构造ElementType实例
			new(&m_Data[posIndex]) ElementType(std::forward<Args>(args)...);
			m_Size++;

			return Iterator(m_Data + posIndex);
		}

		template<typename... Args>
		Iterator EmplaceBack(Args&&... args)
		{
			return Emplace(end(), std::forward<Args>(args)...);
		}

		void Assign(size_t count, const ElementType& value)
		{
			Clear();
			for (size_t i = 0; i != count; i++)
				EmplaceBack(value);
		}

		// 销毁 vector 中所有元素，并用迭代器范围 [first, last) 的内容填充
		void Assign(Iterator first, Iterator last)
		{
#if _DEBUG > 0
			if (first.m_Ptr < begin().m_Ptr || first.m_Ptr >= end().m_Ptr)
				assert("Vector::Assign，first越界");
			if (last.m_Ptr <= begin().m_Ptr || last.m_Ptr > end().m_Ptr)
				assert("Vector::Assign，last越界");
			if (last.m_Ptr - first.m_Ptr < 0)
				assert("Vector::Assign; 迭代器错误");
#endif
			Clear();
			for (; first != last; first++)
				EmplaceBack(*first.m_Ptr);
		}

		// 删除 [first, last) 
		Iterator Erase(ConstIterator first, ConstIterator last)
		{
#if _DEBUG > 0
			if (first > last || first < begin() || last <= begin() || last > end())
				throw std::out_of_range("Vector::Erase; Invalid iterator range");
#endif

			if (first == last) return end();

			const size_t firstIndex = first - begin();
			const size_t lastIndex = last - begin();
			const size_t count = lastIndex - firstIndex;

			for (size_t i = firstIndex; i < lastIndex; i++) 
			{
				m_Data[i].~ElementType();
			}

			for (size_t i = firstIndex; i < m_Size - count; i++)
			{
				//m_Data[i] = std::move(m_Data[i + count]);
				new(&m_Data[i]) ElementType(std::move(m_Data[i + count]));
				m_Data[i + count].~ElementType();
			}

			m_Size -= count;
			return begin() + firstIndex;
		}

		Iterator Erase(ConstIterator pos)
		{
			return Erase(pos, pos + 1);
		}

		void PopBack()
		{
			Erase(--end());
		}

		void Swap(Vector<ElementType>& other)
		{
			if (m_Data == other.m_Data)
				return;

			size_t size       = other.m_Size;
			size_t capacity   = other.m_Capacity;
			ElementType* data = other.m_Data;
			other.m_Size      = m_Size;
			other.m_Capacity  = m_Capacity;
			other.m_Data      = m_Data;
			m_Size            = size;
			m_Capacity        = capacity;
			m_Data            = data;
		}

		void Clear()
		{
			for (size_t i = 0; i < m_Size; i++)
				m_Data[i].~ElementType();
			m_Size = 0;
		}

		void Reserve(size_t newCapacity)
		{
			std::cout << "Reserve: " << newCapacity << std::endl;
			/*
			* PopBack()中调用了元素的析构函数(delete 堆指针)，Vector只调整了m_Size，没有调整m_Data,
			* 在~Vector()中delete[] m_Data过程中调用所有元素（包括PopBack()中的元素）的析构函数，可能会出现重复delete错误
			* 故改变分配和内存的方式，首先将new改为不调用构造的::operator new
			* ~Vector()中，通过Clear()调用m_Size范围内所有元素的析构，再释放m_Capacity范围所有内存
			*/
			ElementType* newBlock = (ElementType*)volstl::BlockAllocator::GetInstance()->Allocate(newCapacity * sizeof(ElementType));

			if (newCapacity < m_Size)
				m_Size = newCapacity;

			for (size_t i = 0; i < m_Size; i++)
				new(&newBlock[i]) ElementType(std::move(m_Data[i]));

			for (size_t i = 0; i < m_Size; i++)
				m_Data[i].~ElementType();

			volstl::BlockAllocator::GetInstance()->Free(m_Data, m_Capacity * sizeof(ElementType));

			m_Data = newBlock;
			m_Capacity = newCapacity;
		}

		void ShrinkToFit()
		{
			Reserve(m_Size);
		}

		void Resize(size_t count, const ElementType& value)
		{
			Reserve(count);
			if (m_Size < count)
				for (size_t i = 0; i != count - m_Size; i++)
					EmplaceBack(value);
		}

		void Resize(size_t count)
		{
			Resize(count, ElementType());
		}

		ElementType& Front()
		{
			return m_Data[0];
		}

		ElementType& Back()
		{
			return m_Data[m_Size - 1];
		}

		ElementType* Data()
		{
			return m_Data;
		}

		const size_t Size() const
		{
			return m_Size;
		}

		const size_t Capacity() const
		{
			return m_Capacity;
		}

		const bool Empty() const
		{
			return m_Size == 0;
		}

		ElementType& operator[](size_t index)
		{
#if _DEBUG > 0
			assert(index < m_Size && "vector subscript out of range");
#endif
			return m_Data[index];
		}

		const ElementType& operator[](size_t index) const
		{
#if _DEBUG > 0
			assert(index < m_Size && "vector subscript out of range");
#endif
			return m_Data[index];
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
			return Iterator(m_Data + m_Size);
		}
		ConstIterator end() const
		{
			return ConstIterator(m_Data + m_Size);
		}

	private:
		Vector(const Vector& other) {}
		Vector(Vector&& other) {}
		Vector& operator=(const Vector& other) {}
		Vector& operator=(Vector&& other) {}
		bool operator==(Vector&& other) {}
		bool operator!=(Vector&& other) {}
		bool operator<(Vector&& other) {}
		bool operator>(Vector&& other) {}
		bool operator<=(Vector&& other) {}
		bool operator>=(Vector&& other) {}

	public:
		ElementType* m_Data = nullptr;
		size_t m_Size = 0;			// 现有长度
		size_t m_Capacity = 0;		// 最大容量
	};



}