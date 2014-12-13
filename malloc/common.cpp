#include "common.h"
#include "linear.h"

#include <sys/mman.h>

namespace alloc
{
    data_block_t::data_block_t(const linear::block_t & block)
        : addr_(block.addr() + sizeof(size_t))
    {}

    data_block_t::data_block_t(ptr_t addr)
        : addr_(addr)
    {}

    ptr_t data_block_t::addr() const
    {
        return addr_;
    }

    linear::block_t data_block_t::to_memory_block() const
    {
        return linear::block_t(addr_ - sizeof(size_t));
    }

    size_t data_block_t::size() const
    {
        return to_memory_block().size() - sizeof(size_t);
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
