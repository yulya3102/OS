#pragma once

#include "common.h"

#include <mutex>

namespace alloc
{
    namespace slab
    {
        struct block_t
        {
            block_t(ptr_t addr);
            block_t(const data_block_t & data_block);
            data_block_t to_data_block() const;
            size_t data_size() const;
            size_t size() const;
            ptr_t addr() const;
            ptr_t next() const;
            ptr_t & next();
            ptr_t bucket_address() const;
        private:
            ptr_t addr_;
        };

        struct bucket_t
        {
            bucket_t(size_t block_size);
            bucket_t(ptr_t addr);
            block_t allocate_block(size_t size);
            void free_block(block_t block);
            bool is_empty();
            size_t header_size() const;
            size_t & block_size();
        private:
            void init(size_t size);
            void add_allocator();
            std::mutex & lock();
            ptr_t & head();
            ptr_t & next_allocator();
            ptr_t & bigger_bucket();
            ptr_t addr_;

            friend struct slab_t;
        };

        struct slab_t
        {
            slab_t(size_t step, size_t max_size);
            block_t allocate_block(size_t size);
            void free_block(block_t block);
        private:
            bucket_t smallest;
        };
    }
}
