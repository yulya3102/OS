#include "hoard.h"

#include <stddef.h>
#include <string.h>
#include <mutex>

namespace
{
    using namespace alloc;

    const size_t SMALL_BLOCK_SIZE = PAGE_SIZE / 4;

    hoard::hoard_t & get_allocator()
    {
        static hoard::hoard_t allocator(sizeof(ptr_t), SMALL_BLOCK_SIZE);
        return allocator;
    }
}

extern "C"
void * malloc(size_t size)
{
    if (!size)
        return NULL;

    return get_allocator().allocate_block(size).addr();
}

extern "C"
void free(void * ptr)
{
    if (!ptr)
        return;

    data_block_t data_block(reinterpret_cast<ptr_t>(ptr));
    get_allocator().free_block(data_block);
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

    data_block_t block(reinterpret_cast<ptr_t>(ptr));
    size_t copied_size = std::min(size, get_allocator().block_size(block));
    void * new_ptr = malloc(size);
    memcpy(new_ptr, ptr, copied_size);
    free(ptr);
    return new_ptr;
}

extern "C"
int posix_memalign(void ** memptr, size_t alignment, size_t size)
{
    if (!size)
    {
        *memptr = nullptr;
        return 0;
    }

    *memptr = get_allocator().allocate_block(size, alignment).addr();
    if (!*memptr)
        return -1;
    return 0;
}

extern "C"
void * memalign(size_t alignment, size_t size)
{
    void * result;
    int r = posix_memalign(&result, alignment, size);
    if (!r)
        return result;
    return nullptr;
}

extern "C"
void * aligned_alloc(size_t alignment, size_t size)
{
    return memalign(alignment, size);
}

extern "C"
void * valloc(size_t size)
{
    return memalign(PAGE_SIZE, size);
}

extern "C"
void * pvalloc(size_t size)
{
    size = (size % PAGE_SIZE == 0) ? size : (size / PAGE_SIZE + 1) * PAGE_SIZE;
    return valloc(size);
}
