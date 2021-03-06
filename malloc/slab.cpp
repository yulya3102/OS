#include "slab.h"
#include "mmap.h"
#include "hoard.h"

#include <cassert>

namespace alloc
{
    namespace slab
    {
        const size_t MAX_SAVED_SLAB_BLOCKS = 20;

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
            return bucket().block_size();
        }

        ptr_t block_t::addr() const
        {
            return addr_;
        }

        ptr_t block_t::next() const
        {
            return *reinterpret_cast<ptr_t *>(addr_);
        }

        ptr_t & block_t::next()
        {
            return *reinterpret_cast<ptr_t *>(addr_);
        }

        bucket_t block_t::bucket() const
        {
            return bucket_t(prev_page_bound(addr_));
        }

        bucket_t::bucket_t(size_t block_size, std::thread::id id)
            : addr_(allocate_pages(1))
        {
            if (addr_)
                init(block_size, id);
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
            if (is_empty())
            {
                if (!next_allocator())
                {
                    add_allocator();
                    if (!next_allocator())
                        return block_t(nullptr);
                }
                block_t new_block = bucket_t(next_allocator()).allocate_block(block_size());
                return new_block;
            }
            else
            {
                block_t block(head());
                head() = block.next();
                block.next() = nullptr;
                return block;
            }
        }

        void bucket_t::free_block(block_t free_block)
        {
            free_block.next() = head();
            head() = free_block.addr();
        }

        bool bucket_t::is_empty()
        {
            return head() == nullptr;
        }

        void bucket_t::init(size_t size, std::thread::id thread_id)
        {
            tag() = tag_t::BUCKET;
            next_allocator() = nullptr;
            block_size() = size;
            head() = addr_ + header_size();
            bigger_bucket() = nullptr;
            id() = thread_id;
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
            bucket_t new_allocator(block_size(), id());
            next_allocator() = new_allocator.addr_;
        }

        tag_t & bucket_t::tag()
        {
            return *reinterpret_cast<tag_t *>(addr_);
        }

        ptr_t & bucket_t::head()
        {
            return *reinterpret_cast<ptr_t *>(&tag() + 1);
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

        std::thread::id & bucket_t::id()
        {
            return *reinterpret_cast<std::thread::id *>(&bigger_bucket() + 1);
        }

        size_t bucket_t::header_size()
        {
            ptr_t not_aligned_head = sizeof(tag_t) + sizeof(std::mutex) + 2 * sizeof(ptr_t) + sizeof(size_t) + sizeof(ptr_t) + sizeof(std::thread::id) + addr_;
            ptr_t aligned_head;
            if (reinterpret_cast<size_t>(not_aligned_head) % block_size() == 0)
                aligned_head = not_aligned_head;
            else
                aligned_head = reinterpret_cast<ptr_t>(
                    (reinterpret_cast<size_t>(not_aligned_head) / block_size() + 1)
                    * block_size());
            return aligned_head - addr_;
        }

        slab_t::slab_t(size_t step, size_t big_size, hoard::hoard_ptr hoard)
            : smallest(sizeof(ptr_t), std::this_thread::get_id())
            , big_size(big_size)
            , id(std::this_thread::get_id())
            , saved_blocks(nullptr)
            , saved_blocks_length(0)
            , hoard(hoard)
        {
            bucket_t bucket = smallest;
            while (bucket.block_size() + step <= big_size)
            {
                bucket_t bigger_bucket = bucket_t(bucket.block_size() + step, id);
                bucket.bigger_bucket() = bigger_bucket.addr_;
                bucket = bigger_bucket;
            }
        }

        data_block_t slab_t::allocate_block(size_t size, size_t alignment)
        {
            if (size <= big_size && alignment != 1)
            {
                for (size_t i = 1; i <= big_size; i *= 2)
                {
                    if (i < size)
                        continue;

                    size = i;
                    break;
                }

                if (size < alignment)
                    size = alignment;
            }

            if (size > big_size)
                return mmap::allocate_block(size, alignment).to_data_block();

            return smallest.allocate_block(size).to_data_block();
        }

        void slab_t::free_block(data_block_t block)
        {
            if (is_mmap_block(block))
                mmap::free_block(block);
            else
            {
                block_t slab_block(block);
                bucket_t allocator = slab_block.bucket();
                if (allocator.id() == id)
                    allocator.free_block(slab_block);
                else
                {
                    *reinterpret_cast<ptr_t *>(block.addr()) = saved_blocks;
                    saved_blocks = block.addr();
                    saved_blocks_length++;
                    if (saved_blocks_length > MAX_SAVED_SLAB_BLOCKS)
                    {
                        hoard->save_slab_blocks(saved_blocks);
                        saved_blocks = nullptr;
                        saved_blocks_length = 0;

                        ptr_t free_blocks = hoard->get_saved_slab_blocks(id);
                        while (free_blocks)
                        {
                            data_block_t block(free_blocks);
                            free_blocks = *reinterpret_cast<ptr_t *>(free_blocks);
                            free_block(block);
                        }
                    }
                }
            }
        }

        size_t slab_t::block_size(const data_block_t & block) const
        {
            if (is_mmap_block(block))
                return mmap::block_t(block).data_size();
            return block_t(block).data_size();
        }

        bool slab_t::is_mmap_block(const data_block_t & block) const
        {
            return *reinterpret_cast<tag_t *>(prev_page_bound(block.addr())) == tag_t::MMAP;
        }
    }
}
