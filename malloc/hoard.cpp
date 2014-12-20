#include "hoard.h"

namespace alloc
{
    namespace hoard
    {
        hoard_t::hoard_t(size_t step, size_t big_size)
            : step(step)
            , big_size(big_size)
        {}

        data_block_t hoard_t::allocate_block(size_t size)
        {
            return get_thread_slab().allocate_block(size);
        }

        void hoard_t::free_block(data_block_t block)
        {
            return get_thread_slab().free_block(block);
        }

        size_t hoard_t::block_size(const data_block_t & block)
        {
            return get_thread_slab().block_size(block);
        }

        slab::slab_t & hoard_t::get_thread_slab()
        {
            thread_local static slab::slab_t slab(step, big_size, this);
            return slab;
        }

        void hoard_t::save_slab_blocks(ptr_t blocks)
        {
            while (blocks)
            {
                ptr_t next = *reinterpret_cast<ptr_t *>(blocks);
                lock.lock();
                *reinterpret_cast<ptr_t *>(blocks) = saved_blocks;
                saved_blocks = blocks;
                lock.unlock();
                blocks = next;
            }
        }

        ptr_t hoard_t::get_saved_slab_blocks(std::thread::id id)
        {
            lock.lock();
            ptr_t result = nullptr;
            ptr_t blocks = saved_blocks;
            ptr_t * prev = &saved_blocks;
            while (blocks)
            {
                if (slab::block_t(blocks).bucket().id() == id)
                {
                    *prev = *reinterpret_cast<ptr_t *>(blocks);
                    *reinterpret_cast<ptr_t *>(blocks) = result;
                    result = blocks;
                }
                prev = reinterpret_cast<ptr_t *>(blocks);
                blocks = *reinterpret_cast<ptr_t *>(blocks);
            }
            lock.unlock();
            return result;
        }
    }
}
