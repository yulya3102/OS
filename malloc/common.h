#pragma once

#include <cstddef>

using ptr_t = unsigned char *;

namespace alloc
{
    const size_t PAGE_SIZE = 4096;

    namespace linear
    {
        struct memory_block_t;
    }

    struct data_block_t
    {
        data_block_t(const linear::memory_block_t & block);
        data_block_t(ptr_t addr);
        ptr_t addr() const;
        linear::memory_block_t to_memory_block() const;
        size_t size() const;
    private:
        ptr_t addr_;
    };
    struct data_block_t;

    size_t bytes_to_pages(size_t bytes);
    linear::memory_block_t allocate_new_block(size_t pages);
}
