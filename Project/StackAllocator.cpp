#include "StackAllocator.hpp"

StackAllocator::StackAllocator(size_t capacity)
    :m_capacity(capacity)
{
    m_base = static_cast<std::uint8_t*>(std::malloc(m_capacity));
    m_offset = 0;
}

StackAllocator::~StackAllocator()
{
    std::free(m_base);
    m_base = nullptr;
    m_capacity = 0;
    m_offset = 0;
}

static size_t AlignUp(size_t value, size_t alignment)
{
    size_t rest = value % alignment;
    if(rest == 0)
        return 0;

    return value + (alignment - rest);
}

void* StackAllocator::Allocate(size_t size, size_t alignment)
{
    size_t alignedOffset = AlignUp(m_offset, alignment);
    if(alignedOffset + size > m_capacity){
        std::cout << "Error: Stack Allocator not enough capacity left" << std::endl;
        return nullptr;
    }

    void* ptr = m_base + alignedOffset;
    m_offset = alignedOffset + size;

    return ptr;
}

void StackAllocator::Reset()
{
    m_offset = 0;
}