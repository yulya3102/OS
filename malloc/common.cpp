#include "common.h"
#include "linear.h"

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

    linear::block_t allocate_new_block(size_t pages)
    {
        size_t size = pages * PAGE_SIZE;
        void * addr = mmap(nullptr, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0);
        if (addr == MAP_FAILED)
            return nullptr;

        linear::block_t result(reinterpret_cast<ptr_t>(addr));
        result.size() = size;
        return result;
    }
}
