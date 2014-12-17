#include "slab.h"

#include <cassert>

namespace alloc
{
    namespace slab
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

        ptr_t block_t::bucket_address() const
        {
            size_t mask = ~(PAGE_SIZE - 1);
            return reinterpret_cast<ptr_t>(reinterpret_cast<size_t>(addr_) & mask);
        }

        bucket_t::bucket_t(size_t block_size)
            : addr_(allocate_pages(1))
        {
            init(block_size);
        }

        bucket_t::bucket_t(ptr_t addr)
            : addr_(addr)
        {}

        block_t bucket_t::allocate_block(size_t size)
        {
            assert(size <= block_size());
            size = block_size();
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
                block_t new_block = bucket_t(next_allocator()).allocate_block(size);
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

        void bucket_t::free_block(block_t free_block)
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

        bool bucket_t::is_empty()
        {
            return head() == nullptr;
        }

        void bucket_t::init(size_t size)
        {
            new (&lock()) std::mutex();
            next_allocator() = nullptr;
            head() = addr_ + header_size();
            block_size() = size;
            block_t block(head());
            block.size() = PAGE_SIZE - header_size();
            block.next() = nullptr;
        }

        void bucket_t::add_allocator()
        {
            bucket_t new_allocator(block_size());
            next_allocator() = new_allocator.addr_;
        }

        std::mutex & bucket_t::lock()
        {
            return *reinterpret_cast<std::mutex *>(addr_);
        }

        ptr_t & bucket_t::head()
        {
            return *reinterpret_cast<ptr_t *>(&lock() + 1);
        }

        ptr_t & bucket_t::next_allocator()
        {
            return *(&head() + 1);
        }

        size_t & bucket_t::block_size()
        {
            return *reinterpret_cast<size_t *>(&next_allocator() + 1);
        }

        size_t bucket_t::header_size() const
        {
            return sizeof(std::mutex) + 2 * sizeof(ptr_t);
        }

        void free_block(block_t block)
        {
            bucket_t allocator(block.bucket_address());
            allocator.free_block(block);
        }
    }
}