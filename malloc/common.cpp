#include "common.h"

#include <sys/mman.h>

namespace alloc
{
    memory_block_t::memory_block_t(ptr_t addr)
        : addr_(addr)
    {}

    size_t memory_block_t::size() const
    {
        return *reinterpret_cast<size_t *>(addr_);
    }

    size_t & memory_block_t::size()
    {
        return *reinterpret_cast<size_t *>(addr_);
    }

    ptr_t memory_block_t::addr() const
    {
        return addr_;
    }

    ptr_t memory_block_t::next() const
    {
        return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
    }

    ptr_t & memory_block_t::next()
    {
        return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
    }

    data_block_t::data_block_t(const memory_block_t & block)
        : addr_(block.addr() + sizeof(size_t))
    {}

    data_block_t::data_block_t(ptr_t addr)
        : addr_(addr)
    {}

    ptr_t data_block_t::addr() const
    {
        return addr_;
    }

    memory_block_t data_block_t::to_memory_block() const
    {
        return memory_block_t(addr_ - sizeof(size_t));
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

    memory_block_t allocate_new_block(size_t pages)
    {
        size_t size = pages * PAGE_SIZE;
        void * addr = mmap(nullptr, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0);
        if (addr == MAP_FAILED)
            return nullptr;

        memory_block_t result(reinterpret_cast<ptr_t>(addr));
        result.size() = size;
        return result;
    }
}
