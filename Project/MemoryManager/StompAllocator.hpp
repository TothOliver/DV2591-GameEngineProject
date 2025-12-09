#pragma once
#include <iostream>
#include <vector>
#include <functional>


class StompAllocator
{
public:
	StompAllocator();
	~StompAllocator();

	void* allocate(size_t size);
	void deallocate(void* ptr);
	//bool accessViolation(std::function<void()> test);

private:
	size_t m_pageSize;

	struct AllocationInfo
	{
		size_t requested_size;
		size_t allocated_pages;
		void* baseAddress;   // The base returned by VirtualAlloc
	};

};
