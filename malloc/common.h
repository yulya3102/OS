#pragma once

#include <cstddef>

using ptr_t = unsigned char *;

namespace alloc
{
    const size_t PAGE_SIZE = 4096;

    namespace linear
    {
        struct block_t;
    }

    struct data_block_t
    {
        data_block_t(const linear::block_t & block);
        data_block_t(ptr_t addr);
        ptr_t addr() const;
        linear::block_t to_memory_block() const;
        size_t size() const;
    private:
        ptr_t addr_;
    };
    struct data_block_t;

    size_t bytes_to_pages(size_t bytes);
    linear::block_t allocate_new_block(size_t pages);
}
