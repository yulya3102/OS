#include <sys/mman.h>

#include <stddef.h>
#include <string.h>
#include <mutex>

namespace
{
    const size_t PAGE_SIZE = 4096;

    using ptr_t = unsigned char *;

    struct memory_block_t
    {
        memory_block_t(ptr_t addr)
            : addr_(addr)
        {}

        size_t size() const
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        size_t & size()
        {
            return *reinterpret_cast<size_t *>(addr_);
        }

        ptr_t addr() const
        {
            return addr_;
        }

        ptr_t next() const
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

        ptr_t & next()
        {
            return *reinterpret_cast<ptr_t *>(addr_ + size() - sizeof(ptr_t));
        }

    private:
        ptr_t addr_;
    };

    struct data_block_t
    {
        data_block_t(const memory_block_t & block)
            : addr_(block.addr() + sizeof(size_t))
        {}

        data_block_t(ptr_t addr)
            : addr_(addr)
        {}

        ptr_t addr() const
        {
            return addr_;
        }

        memory_block_t to_memory_block() const
        {
            return memory_block_t(addr_ - sizeof(size_t));
        }

        size_t size() const
        {
            return to_memory_block().size() - sizeof(size_t);
        }

    private:
        ptr_t addr_;
    };

    size_t bytes_to_pages(size_t bytes)
    {
        if (bytes % PAGE_SIZE)
            return bytes / PAGE_SIZE + 1;
        return bytes / PAGE_SIZE;
    }

    memory_block_t allocate_new_block(size_t pages)
    {
        size_t size = pages * PAGE_SIZE;
        void * addr = mmap(nullptr, size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0);
        if (addr == MAP_FAILED)
            return nullptr;

        memory_block_t result(reinterpret_cast<ptr_t>(addr));
        result.size() = size;
        return result;
    }

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

    memory_block_t block = free_blocks.next_free_block(size + sizeof(size_t));
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
