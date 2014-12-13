#include "linear.h"

namespace alloc
{
    namespace linear
    {
        memory_block_t::memory_block_t(ptr_t addr)
            : addr_(addr)
        {}

        size_t memory_block_t::size() const
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        size_t & memory_block_t::size()
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        ptr_t memory_block_t::addr() const
        {
            return addr_;
        }

        ptr_t memory_block_t::next() const
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

        ptr_t & memory_block_t::next()
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

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
