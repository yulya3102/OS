#include "buffer.h"

#include <cstdlib>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

buffer::buffer(int max_size)
    : buf((char *) malloc(max_size))
    , max_size(max_size)
    , current_size(0)
{}

void buffer::read(int fd) {
    if (current_size < max_size) {
        int r = ::read(fd, buf + current_size, max_size - current_size);
        if (r == -1) {
            throw std::runtime_error(strerror(errno));
        } else if (r == 0) {
            // ???
        } else {
            current_size += r;
        }
    }
}

void buffer::write(int fd) {
    if (current_size > 0) {
        int r = ::write(fd, buf, current_size);
        if (r == -1) {
            throw std::runtime_error(strerror(errno));
        } else {
            memmove(buf, buf + r, current_size - r);
            current_size -= r;
        }
    }
}
