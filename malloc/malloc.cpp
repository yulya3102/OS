#include "common.h"
#include "slab.h"
#include "mmap.h"

#include <stddef.h>
#include <string.h>
#include <mutex>

namespace
{
    using namespace alloc;

    const size_t SMALL_BLOCK_SIZE = PAGE_SIZE / 4;

    slab::slab_t & get_allocator()
    {
        static slab::slab_t allocator(sizeof(ptr_t), SMALL_BLOCK_SIZE);
        return allocator;
    }
}

extern "C"
void * malloc(size_t size)
{
    if (!size)
        return NULL;

    if (size > SMALL_BLOCK_SIZE)
        return mmap::allocate_block(size).to_data_block().addr();

    slab::block_t block = get_allocator().allocate_block(size);
    return block.to_data_block().addr();
}

extern "C"
void free(void * ptr)
{
    if (!ptr)
        return;

    data_block_t data_block(reinterpret_cast<ptr_t>(ptr));
    if ((reinterpret_cast<size_t>(mmap::block_t(data_block).addr()) % PAGE_SIZE) == 0)
        mmap::free_block(data_block);
    else
        get_allocator().free_block(slab::block_t(data_block));
}

extern "C"
void * calloc(size_t nmemb, size_t size)
{
    if (!nmemb || !size)
        return nullptr;

    void * addr = malloc(nmemb * size);
    if (addr)
        return memset(addr, 0, nmemb * size);

    return nullptr;
}

extern "C"
void * realloc(void * ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    size_t copied_size;
    {
        slab::block_t block(data_block_t(reinterpret_cast<ptr_t>(ptr)));
        copied_size = std::min(block.data_size(), size);
    }
    void * new_ptr = malloc(size);
    memcpy(new_ptr, ptr, copied_size);
    free(ptr);
    return new_ptr;
}

extern "C"
int posix_memalign(void ** memptr, size_t alignment, size_t size)
{
    *memptr = nullptr;
    return -1;
}

extern "C"
void * aligned_alloc(size_t alignment, size_t size)
{
    return nullptr;
}

extern "C"
void * valloc(size_t size)
{
    return nullptr;
}

extern "C"
void * memalign(size_t alignment, size_t size)
{
    return nullptr;
}

extern "C"
void * pvalloc(size_t size)
{
    return nullptr;
}
