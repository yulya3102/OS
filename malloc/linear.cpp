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

        size_t block_t::data_size() const
        {
            return size() - sizeof(size_t);
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
            : addr_(allocate_pages(1))
        {
            init();
        }

        linear_allocator_t::linear_allocator_t(ptr_t addr)
            : addr_(addr)
        {}

        block_t linear_allocator_t::allocate_block(size_t size)
        {
            lock().lock();
            ptr_t * prev = &head();
            block_t block(head());
            while (block.addr() && block.size() < size)
            {
                prev = &block.next();
                block = block.next();
            }
            if (!block.addr())
            {
                if (!next_allocator())
                {
                    add_allocator();
                }
                block_t new_block = linear_allocator_t(next_allocator()).allocate_block(size);
                lock().unlock();
                return new_block;
            }
            else
            {
                if (block.size() - size > sizeof(size_t) + sizeof(ptr_t))
                {
                    block_t new_free_block(block.addr() + size);
                    new_free_block.size() = block.size() - size;
                    block.size() = size;
                    block.next() = new_free_block.addr();
                }
                *prev = block.next();
                block.next() = nullptr;
                lock().unlock();
                return block;
            }
        }

        void linear_allocator_t::free_block(block_t free_block)
        {
            lock().lock();
            block_t prev(nullptr), block(head());
            while (block.addr() && block.addr() < free_block.addr())
            {
                prev = block;
                block = block.next();
            }

            if (prev.addr())
                prev.next() = free_block.addr();
            else
                head() = free_block.addr();
            free_block.next() = block.addr();

            if (free_block.addr() + free_block.size() == block.addr())
                free_block.size() += block.size();
            if (prev.addr() && prev.addr() + prev.size() == free_block.addr())
                prev.size() += free_block.size();

            lock().unlock();
        }

        bool linear_allocator_t::is_empty()
        {
            return head() == nullptr;
        }

        void linear_allocator_t::init()
        {
            new (&lock()) std::mutex();
            next_allocator() = nullptr;
            head() = addr_ + header_size();
            block_t block(head());
            block.size() = PAGE_SIZE - header_size();
            block.next() = nullptr;
        }

        void linear_allocator_t::add_allocator()
        {
            linear_allocator_t new_allocator;
            next_allocator() = new_allocator.addr_;
        }

        std::mutex & linear_allocator_t::lock()
        {
            return *reinterpret_cast<std::mutex *>(addr_);
        }

        ptr_t & linear_allocator_t::head()
        {
            return *reinterpret_cast<ptr_t *>(&lock() + 1);
        }

        ptr_t & linear_allocator_t::next_allocator()
        {
            return *(&head() + 1);
        }

        size_t linear_allocator_t::header_size() const
        {
            return sizeof(std::mutex) + 2 * sizeof(ptr_t);
        }
    }
}
