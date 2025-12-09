#pragma once

class PoolAllocator
{
public:
    PoolAllocator(size_t objectSize, size_t objectCount, size_t alignment);
    ~PoolAllocator();

    void* Allocate();
    void Free(void* ptr);
private:
    size_t m_objectSize;
    size_t m_objectCount;
    size_t m_alignment;
    void* m_rawMemory;
    void* m_memoryBlock;
    void* m_freeListHead;
};