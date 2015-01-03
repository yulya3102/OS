#include "common.h"

#include <sys/mman.h>

namespace alloc
{
    data_block_t::data_block_t(ptr_t addr)
        : addr_(addr)
    {}

    ptr_t data_block_t::addr() const
    {
        return addr_;
    }

    size_t bytes_to_pages(size_t bytes)
    {
        if (bytes % PAGE_SIZE)
            return bytes / PAGE_SIZE + 1;
        return bytes / PAGE_SIZE;
    }

    ptr_t allocate_pages(size_t pages)
    {
        size_t size = pages * PAGE_SIZE;
        void * addr = mmap(nullptr, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0);
        if (addr == MAP_FAILED)
            return nullptr;

        return reinterpret_cast<ptr_t>(addr);
    }

    void free_pages(ptr_t addr, size_t pages)
    {
        munmap(addr, pages * PAGE_SIZE);
    }

    ptr_t prev_page_bound(ptr_t addr)
    {
        return reinterpret_cast<ptr_t>(
            reinterpret_cast<size_t>(addr - 1) / PAGE_SIZE * PAGE_SIZE);
    }
}
