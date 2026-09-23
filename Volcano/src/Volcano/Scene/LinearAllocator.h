#pragma once

namespace Volcano
{
    // 线性分配器
    class LinearAllocator
    {
    public:
        LinearAllocator(size_t size)
        {
            m_Data = (uint8_t*)malloc(size);
            m_Size = size;
            Reset();
        }

        ~LinearAllocator()
        {
            free(m_Data);
        }

        void* Allocate(size_t size)
        {
            if (m_Offset + size > m_Size)
            {
                Resize(m_Size * 2 + size);
            }

            void* ptr = m_Data + m_Offset;
            m_Offset += size;
            return ptr;
        }

        // 分配一块size字节的内存，不进行构造，只返回指针和偏移
        // 按 alignment 对齐，通常16
        void* Allocate(size_t size, size_t alignment)
        {
            // alignedOffset 是将 m_Offset 向上对齐到 alignment 的整数倍的结果
            // ~(alignment - 1) 获取掩码，例如16的掩码11110000
            size_t alignedOffset = (m_Offset + alignment - 1) & ~(alignment - 1);

            if (alignedOffset + size > m_Size)
            {
                Resize(m_Size * 2 + size);
            }

            void* ptr = m_Data + alignedOffset;
            m_Offset = alignedOffset + size;
            return ptr;
        }

        void Resize(size_t size)
        {
            int sizeTemp = size < m_Size ? size : m_Size;
            uint8_t* ptr = m_Data;
            m_Data = (uint8_t*)malloc(size);
            memcpy(m_Data, ptr, sizeTemp);
            free(ptr);
            m_Size = size;
        }

        void Reset() { m_Offset = 0; }

        uint8_t* GetBase() const { return m_Data; }

    private:
        uint8_t* m_Data;
        size_t   m_Size;
        size_t   m_Offset; // 当前内存池中，下一个可用空闲位置的字节偏移量
    };
}