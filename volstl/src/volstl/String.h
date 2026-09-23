#pragma once

class String
{
private:
    union {
        char* _ptr;           // 长字符串：堆指针
        char _local[16];      // 短字符串：栈缓冲区（大小可变）
    };
    size_t _size;            // 当前字符串长度
    size_t _capacity;        // 分配的内存容量（不包括结尾的空字符）
};