#pragma once

#include "ListConstIterator.h"

namespace volstl
{
    template<typename T>
    class ListIterator
    {
    public:
        using DataStructure = T;
        using ElementType = typename DataStructure::ElementType;
        using Node = typename DataStructure::Node;
    public:
        ListIterator() : m_Ptr(nullptr) {}
        ListIterator(Node* nodePtr) : m_Ptr(nodePtr) {}
        ~ListIterator() {}

        operator ListConstIterator<DataStructure>() const
        {
            return ListConstIterator<DataStructure>(m_Ptr);
        }

        // 前缀运算符，++it
        ListIterator& operator++()
        {
            m_Ptr = m_Ptr->next;
            return *this;
        }

        // 后缀运算符，it++
        ListIterator operator++(int)
        {
            ListIterator temp = *this;
            m_Ptr = m_Ptr->next;
            return temp;
        }

        // 前缀运算符，--it
        ListIterator& operator--()
        {
            m_Ptr = m_Ptr->prior;
            return *this;
        }

        // 后缀运算符，it--
        ListIterator operator--(int)
        {
            ListIterator temp = *this;
            m_Ptr = m_Ptr->prior;
            return temp;
        }

        ListIterator& operator+=(const int count)
        {
            if (count > 0)
                for (int i = 0; i != count; i++)
                    m_Ptr = m_Ptr->next;
            else
                for (int i = count; i != 0; i++)
                    m_Ptr = m_Ptr->prior;
            return *this;
        }

        ListIterator operator+(const int count) const
        {
            ListIterator iterator = *this;
            iterator += count;
            return iterator;
        }

        ListIterator& operator-=(const int count)
        {
            if (count > 0)
                for (int i = 0; i != count; i++)
                    m_Ptr = m_Ptr->prior;
            else
                for (int i = count; i != 0; i++)
                    m_Ptr = m_Ptr->next;
            return *this;
        }

        ListIterator operator-(const int count) const
        {
            ListIterator iterator = *this;
            iterator -= count;
            return iterator;
        }

        int operator-(const ListIterator& other) const
        {
            int count = 0;
            for (Node* ptr = m_Ptr; ptr != other.m_Ptr; ptr = ptr->next)
            {
                if (ptr->next == m_Ptr)
                    return 0;
                count++;
            }
            return count;
        }

        ElementType& operator*() const
        {
            return m_Ptr->data;
        }

        ElementType* operator->() const
        {
            return &m_Ptr->data;
        }

        bool operator==(const ListIterator& other) const
        {
            return m_Ptr == other.m_Ptr;
        }

        bool operator!=(const ListIterator& other) const
        {
            return !(*this == other);
        }

    private:
        bool operator<(const ListIterator& other) const {}
        bool operator>(const ListIterator& other) const {}
        bool operator<=(const ListIterator& other) const {}
        bool operator>=(const ListIterator& other) const {}

    public:
        Node* m_Ptr;
    };
}