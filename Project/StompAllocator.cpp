#include "StompAllocator.hpp"
#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif
//#define StompDebug
StompAllocator::StompAllocator()
{
#if defined(_WIN32 )
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_pageSize = si.dwPageSize;
#elif defined(__linux__)
	m_pageSize = sysconf(_SC_PAGESIZE);
	printf("%li", m_pageSize);
#endif
}

StompAllocator::~StompAllocator()
{
}

void* StompAllocator::allocate(size_t size)
{

	size_t required_pages = (size + m_pageSize - 1) / m_pageSize; //roof division
	size_t total_pages = 1 + required_pages + 1; //header page + user page + guard page
	size_t total_bytes = m_pageSize * total_pages;

#if defined(_WIN32)
	void* base = VirtualAlloc(nullptr, total_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!base)
	{
		std::cout << "ERROR: Stomp allocation : virtual alloc fail" << std::endl;
		return nullptr;
	}


	DWORD oldprotect = 0;

	char* header_page = (char*)base;
	char* user_ptr = header_page + m_pageSize;
	void* guard_page = (char*)base + (total_pages-1)*m_pageSize; // allign the guard pointer to the end

	
	if (!VirtualProtect(guard_page, m_pageSize, PAGE_NOACCESS, &oldprotect))
	{
		std::cout << "ERROR: Stomp allocation : guard allocation" << std::endl;
		VirtualFree(base, 0, MEM_RELEASE);
		return nullptr;
	}


	AllocationInfo* header = (AllocationInfo*)base;
	header->baseAddress = base;
	header->allocated_pages = total_pages;
	header->requested_size = size;

	if (!VirtualProtect(header_page, m_pageSize, PAGE_READONLY, &oldprotect))
	{
		std::cout << "ERROR: Stomp allocation : protecting header page" << std::endl;
		VirtualFree(base, 0, MEM_RELEASE);
		return nullptr;
	}
#elif defined(__linux__)
	//allocate
	void* base = mmap(nullptr, total_bytes,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0);

	if (base == MAP_FAILED) {
		std::cout << "ERROR: mmap failed\n";
		return nullptr;
	}


	char* header_page = (char*)base;
	char* user_ptr = header_page + m_pageSize;
	void* guard_page = (char*)base + (total_pages - 1) * m_pageSize; // allign the guard pointer to the end
	
	if (mprotect(guard_page, m_pageSize, PROT_NONE) != 0) {
		std::cout << "ERROR: mprotect guard failed\n";
		munmap(base, total_bytes);
		return nullptr;
	}

	AllocationInfo* header = (AllocationInfo*)header_page;
	header->baseAddress = base;
	header->allocated_pages = total_pages;
	header->requested_size = size;

	// Protect header page as READONLY
	if (mprotect(header_page, m_pageSize, PROT_READ) != 0) {
		std::cout << "ERROR: mprotect header failed\n";
		munmap(base, total_bytes);
		return nullptr;
	}
#endif

#ifdef StompDebug
	std::cout << "base:        " << base << "\n";
	std::cout << "user_ptr:    " << (void*)user_ptr << "\n";
	std::cout << "guard_page:  " << guard_page << "\n";
	std::cout << "total_bytes: " << total_bytes << "\n";
	std::cout << "required_pages: " << required_pages << "\n";
	std::cout << "total_pages: " << total_pages << "\n";
#endif
	
	return user_ptr;
}

void StompAllocator::deallocate(void* ptr)
{
	if (!ptr)
	{
		std::cout << "ERROR: stomp allocator: deallocate ptr not found\n";
	}
	char* header_page = (char*)ptr - m_pageSize;

	AllocationInfo* header = (AllocationInfo*)header_page;
	void* base = header->baseAddress;
	size_t total_pages = header->allocated_pages;
	size_t total_bytes = total_pages * m_pageSize;

#if defined(_WIN32)

	VirtualFree(base, 0, MEM_RELEASE);
		
#elif defined(__linux__)
	munmap(base, total_bytes);
#endif
	
}


