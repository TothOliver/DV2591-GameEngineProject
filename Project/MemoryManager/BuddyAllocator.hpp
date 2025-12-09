#pragma once
#include <iostream>
#include <vector>

class BuddyAllocator
{
public:
	BuddyAllocator(size_t minBlockSize, size_t totalSize);
	~BuddyAllocator();

	void* Allocate(size_t size);
	void Deallocate(void* ptr);

private:
	size_t m_minBlockSize;	//Smallest allowed block
	size_t m_totalSize;		//total size of all blocks
	void* m_basePtr;		//raw memory block

	std::vector<std::vector<void*>> m_freeblocks;
	std::vector<int> m_blocklevel;

	int GetLevelForSize(size_t size);
	size_t GetBlockSizeForLevel(size_t level);
	void Split(int level);
	void Merge(int level, int offset);
};
