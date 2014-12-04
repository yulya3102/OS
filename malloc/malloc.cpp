#include <sys/mman.h>

#include <stddef.h>
#include <string.h>

extern "C"
void * malloc(size_t size)
{
    if (!size)
        return NULL;

    size_t * addr = reinterpret_cast<size_t *>(mmap(NULL, size + sizeof(size_t),
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE,
                         -1, 0));
    if (addr == MAP_FAILED)
        return NULL;

    *addr = size;
    return addr + 1;
}

extern "C"
void free(void * ptr)
{
    if (!ptr)
        return;

    size_t * addr = reinterpret_cast<size_t *>(ptr);
    addr--;
    munmap(addr, *addr);
}

extern "C"
void * calloc(size_t nmemb, size_t size)
{
    if (!nmemb || !size)
        return NULL;

    void * addr = malloc(nmemb * size);
    if (addr)
        return memset(addr, 0, nmemb * size);

    return NULL;
}

extern "C"
void * realloc(void * ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    void * new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    size_t old_size = *(((size_t *) ptr) - 1);
    size_t n = (size < old_size) ? size : old_size;

    return memcpy(new_ptr, ptr, n);
}
