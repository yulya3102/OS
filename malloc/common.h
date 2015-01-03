#pragma once

#include <cstddef>

using ptr_t = unsigned char *;

namespace alloc
{
    const size_t PAGE_SIZE = 4096;

    struct data_block_t
    {
        data_block_t(ptr_t addr);
        ptr_t addr() const;
    private:
        ptr_t addr_;
    };

    size_t bytes_to_pages(size_t bytes);
    ptr_t allocate_pages(size_t pages);
    void free_pages(ptr_t addr, size_t pages);

    enum class tag_t : char
    {
        BUCKET,
        MMAP
    };

    ptr_t prev_page_bound(ptr_t addr);
}
