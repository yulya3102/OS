#include <sys/mman.h>

#include <stddef.h>
#include <string.h>

void * malloc(size_t size)
{
    if (!size)
        return NULL;

    size_t * addr = mmap(NULL, size + sizeof(size_t),
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE,
                         -1, 0);
    if (addr == MAP_FAILED)
        return NULL;

    *addr = size;
    return addr + 1;
}

void free(void * ptr)
{
    if (!ptr)
        return;

    size_t * addr = ptr;
    addr--;
    munmap(addr, *addr);
}

void * calloc(size_t nmemb, size_t size)
{
    if (!nmemb || !size)
        return NULL;

    void * addr = malloc(nmemb * size);
    if (addr)
        return memset(addr, 0, nmemb * size);

    return NULL;
}

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
