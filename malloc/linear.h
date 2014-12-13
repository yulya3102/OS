#pragma once

#include "common.h"

#include <mutex>

namespace alloc
{
    namespace linear
    {
        struct linear_allocator_t
        {
            linear_allocator_t();
            memory_block_t allocate_block(size_t size);
            void free_block(memory_block_t block);
        private:
            std::mutex lock;
            ptr_t head;
        };
    }
}
