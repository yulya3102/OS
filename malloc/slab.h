#pragma once

#include "common.h"
#include "hoard_fwd.h"

#include <mutex>
#include <thread>

namespace alloc
{
    namespace slab
    {
        struct bucket_t;

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
            bucket_t bucket() const;
        private:
            ptr_t addr_;
        };

        struct bucket_t
        {
            bucket_t(size_t block_size, std::thread::id id);
            bucket_t(ptr_t addr);
            block_t allocate_block(size_t size);
            void free_block(block_t block);
            bool is_empty();
            size_t & block_size();
        private:
            void init(size_t size, std::thread::id id);
            void add_allocator();
            std::mutex & lock();
            ptr_t & head();
            ptr_t & next_allocator();
            ptr_t & bigger_bucket();
            std::thread::id & id();
            size_t header_size();
            ptr_t addr_;

            friend struct slab_t;
        };

        struct slab_t
        {
            slab_t(size_t step, size_t big_size, hoard::hoard_ptr hoard);
            data_block_t allocate_block(size_t size);
            void free_block(data_block_t block);
            size_t block_size(const data_block_t & block) const;
        private:
            bool is_mmap_block(const data_block_t & block) const;
            bucket_t smallest;
            size_t big_size;
            std::thread::id id;
            ptr_t saved_blocks;
            size_t saved_blocks_length;
            hoard::hoard_ptr hoard;
        };
    }
}
