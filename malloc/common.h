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
        data_block_t(ptr_t addr);
        ptr_t addr() const;
    private:
        ptr_t addr_;
    };

    size_t bytes_to_pages(size_t bytes);
    ptr_t allocate_pages(size_t pages);
}
