#include "autofd.h"
#include <algorithm>
#include <unistd.h>

autofd::autofd(int fd)
    : fd(fd)
{}

autofd::autofd(autofd && other) {
    fd = -1;
    std::swap(fd, other.fd);
}

autofd& autofd::operator=(autofd && other) {
    std::swap(fd, other.fd);
    return *this;
}

const int& autofd::operator*() const {
    return fd;
}

autofd::~autofd() {
    if (fd != -1) {
        close(fd);
    }
}
