#include "linear.h"

namespace alloc
{
    namespace linear
    {
        linear_allocator_t::linear_allocator_t()
            : head(nullptr)
        {}

        memory_block_t linear_allocator_t::allocate_block(size_t size)
        {
            lock.lock();
            ptr_t * prev = &head;
            memory_block_t block(head);
            while (block.addr() && block.size() < size)
            {
                prev = &block.next();
                block = block.next();
            }
            if (!block.addr())
            {
                lock.unlock();
                return allocate_new_block(bytes_to_pages(size));
            }
            else
            {
                *prev = block.next();
                block.next() = nullptr;
                lock.unlock();
                return block;
            }
        }

        void linear_allocator_t::free_block(memory_block_t block)
        {
            lock.lock();
            block.next() = head;
            head = block.addr();
            lock.unlock();
        }
    }
}
