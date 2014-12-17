#pragma once

#include "common.h"

#include <mutex>

namespace alloc
{
    namespace linear
    {
        struct block_t
        {
            block_t(ptr_t addr);
            block_t(const data_block_t & data_block);
            data_block_t to_data_block() const;
            size_t data_size() const;
            size_t size() const;
            size_t & size();
            ptr_t addr() const;
            ptr_t next() const;
            ptr_t & next();
        private:
            ptr_t addr_;
        };

        struct linear_allocator_t
        {
            linear_allocator_t();
            linear_allocator_t(ptr_t addr);
            block_t allocate_block(size_t size);
            void free_block(block_t block);
            bool is_empty();
        private:
            void init();
            std::mutex & lock();
            ptr_t & head();
            ptr_t & next_allocator();
            size_t header_size();
            ptr_t addr_;
        };
    }
}
