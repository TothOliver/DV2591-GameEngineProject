#include "BuddyAllocator.hpp"

BuddyAllocator::BuddyAllocator(size_t minBlockSize, size_t totalSize)
	:m_minBlockSize(minBlockSize), m_totalSize(totalSize)
{
	m_basePtr = malloc(totalSize);
	int maxLevel = GetLevelForSize(totalSize);
	m_freeblocks.resize(maxLevel + 1);
	m_freeblocks[maxLevel].push_back(m_basePtr);
	m_blocklevel.resize(m_totalSize / m_minBlockSize);
}

BuddyAllocator::~BuddyAllocator()
{
	free(m_basePtr);
}

void* BuddyAllocator::Allocate(size_t size)
{
	if (size > m_totalSize) return nullptr;

	int level = GetLevelForSize(size);
	Split(level);
	if (m_freeblocks[level].empty())
	{
		//no more memory to give :(
		return nullptr;
	}
	void* blkptr = m_freeblocks[level].back();
	m_freeblocks[level].pop_back();

	//add to list
	size_t offset = (char*)blkptr - (char*)m_basePtr;
	m_blocklevel[offset / m_minBlockSize] = level;

	//Get the level needed for allocation size
	//Run split
	//Check if still empty (no memory left)
	//if not empty pop and return pointer

	return blkptr;
}

void BuddyAllocator::Deallocate(void* ptr)
{
	size_t offset = (char*)ptr - (char*)m_basePtr;
	int level = m_blocklevel[offset / m_minBlockSize];
	Merge(level, offset);
}

int BuddyAllocator::GetLevelForSize(size_t size)
{
	int level = 0;
	int blocksize = GetBlockSizeForLevel(level);
	while (size > blocksize)
	{
		level += 1;
		blocksize = GetBlockSizeForLevel(level);
	}
	return level;
}

size_t BuddyAllocator::GetBlockSizeForLevel(size_t level)
{
	size_t size = m_minBlockSize;
	for (int i = 0; i < level; i++)
	{
		size = size * 2;
	}
	return size;
}

void BuddyAllocator::Split(int level)
{
	if (!m_freeblocks[level].empty())
	{
		return;
	}

	int maxlevel = m_freeblocks.size() - 1;
	if (level >= maxlevel)
	{
		//out of memory
		return;
	}

	Split(level + 1);

	if (m_freeblocks[level + 1].empty())
	{
		//cannot split parent is also out of memory
		return;
	}

	void* parentBlk = m_freeblocks[level + 1].back();
	m_freeblocks[level + 1].pop_back();
	size_t halfPoint = GetBlockSizeForLevel(level);

	char* child1 = (char*)parentBlk;
	char* child2 = child1 + halfPoint;

	m_freeblocks[level].push_back(child1);
	m_freeblocks[level].push_back(child2);

	//check if free block exists on level (if yes return, if no continue)
	//If we are at highest level no more memory
	//recurssion on level + 1
	//pop one parent block
	//add 2 children blocks
}

void BuddyAllocator::Merge(int level, int offset)
{
    int currentLevel = level;
    int currentOffset = offset;

    while (currentLevel < m_freeblocks.size() - 1)
    {
        size_t blockSize = GetBlockSizeForLevel(currentLevel);

        //Compute buddy offset using XOR trick
        int buddyOffset = currentOffset ^ blockSize;
        void* buddyPtr = (char*)m_basePtr + buddyOffset;

        bool buddyFree = false;
        auto& list = m_freeblocks[currentLevel];

        for (int i = 0; i < list.size(); i++)
        {
            if (list[i] == buddyPtr)
            {
                buddyFree = true;
                list[i] = list.back();
                list.pop_back();
                break;
            }
        }

        if (!buddyFree)
        {
            //Buddy isnt free stop
            m_freeblocks[currentLevel].push_back((char*)m_basePtr + currentOffset);
            return;
        }

        int parentOffset = std::min(currentOffset, buddyOffset);

        //Next level
        currentLevel++;
        currentOffset = parentOffset;
    }

    //Insert final merged block
    m_freeblocks[currentLevel].push_back((char*)m_basePtr + currentOffset);
}
