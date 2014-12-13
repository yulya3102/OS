#include "common.h"
#include "linear.h"

#include <stddef.h>
#include <string.h>
#include <mutex>

namespace
{
    using namespace alloc;

    linear::linear_allocator_t allocator;
}

extern "C"
void * malloc(size_t size)
{
    if (!size)
        return NULL;

    size_t real_size = ((size > sizeof(ptr_t)) ? size : sizeof(ptr_t)) + sizeof(size_t);
    linear::memory_block_t block = allocator.allocate_block(real_size);
    if (block.size() - real_size > sizeof(size_t) + sizeof(ptr_t))
    {
        linear::memory_block_t free_block(block.addr() + real_size);
        free_block.size() = block.size() - real_size;
        block.size() = real_size;
        block.next() = free_block.addr();
        allocator.free_block(free_block);
    }
    return data_block_t(block).addr();
}

extern "C"
void free(void * ptr)
{
    if (!ptr)
        return;

    data_block_t data_block(reinterpret_cast<ptr_t>(ptr));
    allocator.free_block(data_block.to_memory_block());
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

    void * new_ptr = malloc(size);
    memcpy(new_ptr, ptr, size);
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
