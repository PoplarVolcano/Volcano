#pragma once

#include "Volcano/Core/Base.h"

namespace Volcano {

	struct Buffer
	{
		byte* Data;
		uint32_t Size;

		Buffer() : Data(nullptr), Size(0) {}
		Buffer(byte* data, uint32_t size) : Data(data), Size(size) { }

		static Buffer Copy(void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}
		// 清除数据并获取size大小的内存
		void Allocate(uint32_t size)
		{
			delete[] Data;
			Data = nullptr;
			if (size == 0)
				return;
			Data = new byte[size];
			Size = size;
		}
		// 数据置零
		void ZeroInitialize() { if (Data) memset(Data, 0, Size); }

		void Write(void* data, uint32_t size, uint32_t offset = 0) { memcpy(Data + offset, data, size); }

		operator bool() const { return Data; }

		byte& operator[](int index) { return Data[index]; }

		byte operator[](int index) const { return Data[index]; }

		template<typename T>
		T* As() { return (T*)Data; }

		inline uint32_t GetSize() const { return Size; }
	};

}