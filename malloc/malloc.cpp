#include "common.h"

#include <stddef.h>
#include <string.h>
#include <mutex>

namespace
{
    using namespace alloc;

    struct blocks_list_t
    {
        blocks_list_t()
            : head(nullptr)
        {}

        memory_block_t next_free_block(size_t size)
        {
            head_lock.lock();
            ptr_t * prev = &head;
            memory_block_t block(head);
            while (block.addr() && block.size() < size)
            {
                prev = &block.next();
                block = block.next();
            }
            if (!block.addr())
            {
                head_lock.unlock();
                return allocate_new_block(bytes_to_pages(size));
            }
            else
            {
                *prev = block.next();
                block.next() = nullptr;
                head_lock.unlock();
                return block;
            }
        }

        void free_block(memory_block_t block)
        {
            head_lock.lock();
            block.next() = head;
            head = block.addr();
            head_lock.unlock();
        }

        std::mutex head_lock;
        ptr_t head;
    };

    blocks_list_t free_blocks;
}

extern "C"
void * malloc(size_t size)
{
    if (!size)
        return NULL;

    size_t real_size = ((size > sizeof(ptr_t)) ? size : sizeof(ptr_t)) + sizeof(size_t);
    memory_block_t block = free_blocks.next_free_block(real_size);
    if (block.size() - real_size > sizeof(size_t) + sizeof(ptr_t))
    {
        memory_block_t free_block(block.addr() + real_size);
        free_block.size() = block.size() - real_size;
        block.size() = real_size;
        block.next() = free_block.addr();
        free_blocks.free_block(free_block);
    }
    return data_block_t(block).addr();
}

extern "C"
void free(void * ptr)
{
    if (!ptr)
        return;

    data_block_t data_block(reinterpret_cast<ptr_t>(ptr));
    free_blocks.free_block(data_block.to_memory_block());
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
