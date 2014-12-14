#include "linear.h"

namespace alloc
{
    namespace linear
    {
        block_t::block_t(ptr_t addr)
            : addr_(addr)
        {}

        block_t::block_t(const data_block_t & data_block)
            : addr_(data_block.addr() - sizeof(size_t))
        {}

        data_block_t block_t::to_data_block() const
        {
            return data_block_t(addr_ + sizeof(size_t));
        }

        size_t block_t::size() const
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        size_t & block_t::size()
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        ptr_t block_t::addr() const
        {
            return addr_;
        }

        ptr_t block_t::next() const
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

        ptr_t & block_t::next()
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

        linear_allocator_t::linear_allocator_t()
            : head(nullptr)
        {}

        block_t linear_allocator_t::allocate_block(size_t size)
        {
            lock.lock();
            ptr_t * prev = &head;
            block_t block(head);
            while (block.addr() && block.size() < size)
            {
                prev = &block.next();
                block = block.next();
            }
            if (!block.addr())
            {
                lock.unlock();
                size_t pages = bytes_to_pages(size);
                block_t new_block(allocate_pages(pages));
                new_block.size() = pages * PAGE_SIZE;
                return new_block;
            }
            else
            {
                *prev = block.next();
                block.next() = nullptr;
                lock.unlock();
                return block;
            }
        }

        void linear_allocator_t::free_block(block_t free_block)
        {
            lock.lock();
            ptr_t * prev = &head;
            block_t prev_block(nullptr);
            block_t block(head);
            while (block.addr() && block.addr() < free_block.addr())
            {
                prev = &block.next();
                prev_block = block;
                block = block.next();
            }
            *prev = free_block.addr();
            free_block.next() = block.addr();
            if (free_block.addr() + free_block.size() == block.addr())
                free_block.size() += block.size();
            if (prev_block.addr() && prev_block.addr() + prev_block.size() == free_block.addr())
                prev_block.size() += free_block.size();
            lock.unlock();
        }
    }
}
