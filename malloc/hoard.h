#pragma once

#include "common.h"
#include "slab.h"

#include <mutex>

namespace alloc
{
    namespace hoard
    {
        struct hoard_t
        {
            hoard_t(size_t step, size_t big_size);
            data_block_t allocate_block(size_t size);
            void free_block(data_block_t block);
            size_t block_size(const data_block_t & block);
            void save_slab_blocks(ptr_t blocks);
        private:
            slab::slab_t & get_thread_slab();
            size_t step, big_size;
            std::mutex lock;
            ptr_t saved_blocks;
        };
    }
}
