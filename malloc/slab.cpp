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
            : addr_(data_block.addr())
        {}

        data_block_t block_t::to_data_block() const
        {
            return data_block_t(addr_);
        }

        size_t block_t::data_size() const
        {
            return size();
        }

        size_t block_t::size() const
        {
            return bucket_t(bucket_address()).block_size();
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
            if (size > block_size())
            {
                assert(bigger_bucket());
                return bucket_t(bigger_bucket()).allocate_block(size);
            }
            lock().lock();
            if (is_empty())
            {
                if (!next_allocator())
                {
                    add_allocator();
                }
                block_t new_block = bucket_t(next_allocator()).allocate_block(block_size());
                lock().unlock();
                return new_block;
            }
            else
            {
                block_t block(head());
                head() = block.next();
                block.next() = nullptr;
                lock().unlock();
                return block;
            }
        }

        void bucket_t::free_block(block_t free_block)
        {
            lock().lock();
            free_block.next() = head();
            head() = free_block.addr();
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
            bigger_bucket() = nullptr;
            block_t block(head());
            size_t unsplitted_size = PAGE_SIZE - header_size();
            while (true)
            {
                block_t next_block(block.addr() + block_size());
                unsplitted_size -= block_size();
                if (unsplitted_size < block_size())
                {
                    block.next() = nullptr;
                    break;
                }
                block.next() = next_block.addr();
                block = next_block;
            }
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

        ptr_t & bucket_t::bigger_bucket()
        {
            return *reinterpret_cast<ptr_t *>(&block_size() + 1);
        }

        size_t bucket_t::header_size() const
        {
            return sizeof(std::mutex) + 2 * sizeof(ptr_t) + sizeof(size_t) + sizeof(ptr_t);
        }

        void free_block(block_t block)
        {
            bucket_t allocator(block.bucket_address());
            allocator.free_block(block);
        }

        slab_t::slab_t(size_t step, size_t max_size)
            : smallest(sizeof(ptr_t))
        {
            bucket_t bucket = smallest;
            while (bucket.block_size() + step <= max_size)
            {
                bucket_t bigger_bucket = bucket_t(bucket.block_size() + step);
                bucket.bigger_bucket() = bigger_bucket.addr_;
                bucket = bigger_bucket;
            }
        }

        block_t slab_t::allocate_block(size_t size)
        {
            return smallest.allocate_block(size);
        }

        void slab_t::free_block(block_t block)
        {
            free_block(block);
        }
    }
}
