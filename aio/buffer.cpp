#include "buffer.h"

#include <cstdlib>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

buffer::buffer(int fd, int max_size)
    : buf((char *) malloc(max_size))
    , max_size(max_size)
    , current_size(0)
    , fd(fd)
    , eof(false)
{}

void buffer::read() {
    if (!eof && current_size < max_size) {
        int r = ::read(fd, buf + current_size, max_size - current_size);
        if (r == -1) {
            throw std::runtime_error(strerror(errno));
        } else if (r == 0) {
            eof = true;
        } else {
            current_size += r;
        }
    }
}

bool buffer::is_eof() {
    return eof;
}
