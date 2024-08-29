#pragma once

#include "Volcano/Core/Base.h"

namespace Volcano {

	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;
		Buffer(uint64_t size) { Allocate(size); }
		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer other)
		{
			Buffer buffer(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}
		// 清除数据并获取size大小的内存
		void Allocate(uint64_t size)
		{
			Release();
			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		/*
		// 数据置零
		void ZeroInitialize() { if (Data) memset(Data, 0, Size); }

		void Write(void* data, uint32_t size, uint32_t offset = 0) { memcpy(Data + offset, data, size); }

		uint8_t& operator[](int index) { return Data[index]; }

		uint8_t operator[](int index) const { return Data[index]; }
		*/
		operator bool() const { return (bool)Data; }

		template<typename T>
		T* As() { return (T*)Data; }

		inline uint64_t GetSize() const { return Size; }
	};


	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer)
			: m_Buffer(buffer)
		{
		}

		ScopedBuffer(uint64_t size)
			: m_Buffer(size)
		{
		}

		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.Data; }
		uint64_t Size() { return m_Buffer.Size; }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }
	private:
		Buffer m_Buffer;
	};
}