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

        size_t hoard_t::block_size(const data_block_t & block) const
        {
            return get_thread_slab().block_size(block);
        }

        slab::slab_t & hoard_t::get_thread_slab() const
        {
            thread_local static slab::slab_t slab(step, big_size);
            return slab;
        }
    }
}
