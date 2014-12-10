#pragma once

#include <cstddef>

using ptr_t = unsigned char *;

namespace alloc
{
    const size_t PAGE_SIZE = 4096;

    struct memory_block_t
    {
        memory_block_t(ptr_t addr);
        size_t size() const;
        size_t & size();
        ptr_t addr() const;
        ptr_t next() const;
        ptr_t & next();
    private:
        ptr_t addr_;
    };

    struct data_block_t
    {
        data_block_t(const memory_block_t & block);
        data_block_t(ptr_t addr);
        ptr_t addr() const;
        memory_block_t to_memory_block() const;
        size_t size() const;
    private:
        ptr_t addr_;
    };
    struct data_block_t;

    size_t bytes_to_pages(size_t bytes);
    memory_block_t allocate_new_block(size_t pages);
}
