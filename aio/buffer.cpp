#include "buffer.h"

#include <cstdlib>
#include <unistd.h>

buffer::buffer(int fd, int max_size)
    : buf((char *) malloc(max_size))
    , max_size(max_size)
    , current_size(0)
    , fd(fd)
{}

void buffer::read() {
    int r = ::read(fd, buf + current_size, max_size - current_size);
    if (r == -1) {
        // ???
    } else if (r == 0) {
        // ???
    } else {
        current_size += r;
    }
}
