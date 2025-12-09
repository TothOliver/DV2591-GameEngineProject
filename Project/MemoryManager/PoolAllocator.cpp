#include <iostream>
#include "PoolAllocator.hpp"

PoolAllocator::PoolAllocator(size_t objectSize, size_t objectCount, size_t alignment)
    : m_objectSize(objectSize),
    m_objectCount(objectCount),
    m_alignment(alignment),
    m_rawMemory(nullptr),
    m_memoryBlock(nullptr),
    m_freeListHead(nullptr)
{

    m_objectSize = (objectSize + (alignment - 1)) & ~(alignment - 1);

    if (m_objectSize < sizeof(void*)) {
        m_objectSize = sizeof(void*);
    }
    
    size_t totalSize = m_objectSize * objectCount;

    m_rawMemory = std::malloc(totalSize + alignment);
    if (!m_rawMemory) {
        throw std::bad_alloc();
    }

    std::uintptr_t rawAddr = reinterpret_cast<std::uintptr_t>(m_rawMemory);
    std::uintptr_t alignedAddr = (rawAddr + (alignment - 1)) & ~(alignment - 1);
    m_memoryBlock = reinterpret_cast<void*>(alignedAddr);

    char* current = static_cast<char*>(m_memoryBlock);

    for (size_t i = 0; i < objectCount - 1; ++i)
    {
        void* next = current + m_objectSize;
        *reinterpret_cast<void**>(current) = next;
        current += m_objectSize;
    }

    *reinterpret_cast<void**>(current) = nullptr;
    m_freeListHead = m_memoryBlock;
}

PoolAllocator::~PoolAllocator()
{
    std::free(m_rawMemory);
}

void* PoolAllocator::Allocate()
{
    if (m_freeListHead == nullptr)
        return nullptr;

    void* allocated = m_freeListHead;

    m_freeListHead = *reinterpret_cast<void**>(m_freeListHead);

    return allocated;
}

void PoolAllocator::Free(void* ptr)
{
    if (!ptr)
        return;

    *reinterpret_cast<void**>(ptr) = m_freeListHead;
    m_freeListHead = ptr;
}