#include "mmap.h"
#include "common.h"

namespace alloc
{
    namespace mmap
    {
        block_t::block_t(ptr_t addr, size_t offset)
            : addr_(addr)
            , offset_(offset)
        {
            tag() = tag_t::MMAP;
        }

        block_t::block_t()
            : addr_(nullptr)
            , offset_(0)
        {}

        block_t::block_t(const data_block_t & data_block)
            : addr_(prev_page_bound(data_block.addr()))
            , offset_(data_block.addr() - addr_)
        {}

        data_block_t block_t::to_data_block() const
        {
            if (addr_)
                return data_block_t(addr_ + offset_);
            return data_block_t(nullptr);
        }

        size_t block_t::data_size()
        {
            if (addr_)
                return size() - offset_;
            return 0;
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
            size_t offset = sizeof(size_t) + sizeof(tag_t);
            size_t pages = bytes_to_pages(size + offset);
            block_t block(allocate_pages(pages), offset);
            block.size() = pages * PAGE_SIZE;
            return block;
        }

        void free_block(block_t block)
        {
            free_pages(block.addr(), bytes_to_pages(block.size()));
        }
    }
}
