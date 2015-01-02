#include "mmap.h"
#include "common.h"

namespace alloc
{
    namespace mmap
    {
        block_t::block_t(ptr_t addr)
            : addr_(addr)
        {
            tag() = tag_t::MMAP;
        }

        block_t::block_t(const data_block_t & data_block)
            : addr_(data_block.addr() - sizeof(size_t) - sizeof(tag_t))
        {}

        data_block_t block_t::to_data_block() const
        {
            return data_block_t(addr_ + sizeof(size_t) + sizeof(tag_t));
        }

        size_t block_t::data_size()
        {
            return size() - sizeof(size_t) - sizeof(tag_t);
        }

        tag_t & block_t::tag()
        {
            return *reinterpret_cast<tag_t *>(addr_);
        }

        size_t & block_t::size()
        {
            return *reinterpret_cast<size_t *>(&tag() + 1);
        }

        ptr_t block_t::addr() const
        {
            return addr_;
        }

        block_t allocate_block(size_t size, size_t alignment)
        {
            size_t pages = bytes_to_pages(size + sizeof(size_t) + sizeof(tag_t));
            block_t block(allocate_pages(pages));
            block.size() = pages * PAGE_SIZE;
            return block;
        }

        void free_block(block_t block)
        {
            free_pages(block.addr(), bytes_to_pages(block.size()));
        }
    }
}
