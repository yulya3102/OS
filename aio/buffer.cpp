#include "buffer.h"

#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

static void * malloc_(int size) {
    void * ptr = malloc(size);
    if (size != 0 && ptr == NULL) {
        throw std::runtime_error("malloc failed");
    }
    return ptr;
}

buffer::buffer(int max_size)
    : max_size(max_size)
    , buf((char *) malloc_(max_size))
{}

void buffer::read(int fd) {
    read(fd, max_size);
}

void buffer::read(int fd, int size) {
    size = std::min(size, max_size);
    if (*current_size < size) {
        int r = ::read(fd, buf + *current_size, size - *current_size);
        if (r == -1) {
            throw std::runtime_error(strerror(errno));
        } else if (r == 0) {
            // ???
        } else {
            current_size = *current_size + r;
        }
    }
}

void buffer::write(int fd) {
    if (*current_size > 0) {
        int r = ::write(fd, buf, *current_size);
        if (r == -1) {
            throw std::runtime_error(strerror(errno));
        } else {
            memmove(buf, buf + r, *current_size - r);
            current_size = *current_size - r;
        }
    }
}

var<int>& buffer::size() {
    return current_size;
}

buffer::~buffer() {
    free(buf);
}
