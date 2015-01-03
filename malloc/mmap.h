#pragma once

#include "common.h"

namespace alloc
{
    namespace mmap
    {
        struct block_t
        {
            block_t(ptr_t addr);
            block_t();
            block_t(const data_block_t & data_block);
            data_block_t to_data_block() const;
            size_t data_size();
            tag_t & tag();
            size_t & size();
            ptr_t addr() const;
        private:
            ptr_t addr_;
        };

        block_t allocate_block(size_t size, size_t alignment);
        void free_block(block_t block);
    }
}
