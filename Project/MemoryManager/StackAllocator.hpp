#pragma once
#include <iostream>
class StackAllocator
{
public:
    StackAllocator(size_t capacity);
    ~StackAllocator();

    void* Allocate(size_t size, size_t alignment = alignof(::max_align_t));
    void Reset();

private:
    std::uint8_t* m_base;
    size_t m_capacity;
    size_t m_offset;

};