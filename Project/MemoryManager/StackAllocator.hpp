#pragma once
#include <iostream>
class StackAllocator
{
public:
    StackAllocator(size_t capacity);
    ~StackAllocator();

    void* Allocate(size_t size, size_t alignment = alignof(::max_align_t));
    void Reset();

    size_t GetUsed() const { return m_offset; }
    size_t GetCapacity() const { return m_capacity; }
    float GetUsageRatio() const
    {
        if(m_capacity == 0) return 0.0f;
        return static_cast<float>(m_offset) / static_cast<float>(m_capacity);
    }

private:
    std::uint8_t* m_base;
    size_t m_capacity;
    size_t m_offset;

    size_t m_peak = 0;
};