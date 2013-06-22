#include "aoperation.h"
#include "epollfd.h"

aoperation::aoperation(epollfd& e, int event, int fd)
    : valid(true)
    , e(&e)
    , event(event)
    , fd(fd)
{}

aoperation::aoperation(aoperation && other)
    : valid(false)
    , e(nullptr)
    , event(0)
    , fd(-1) {
    swap(other);
}

aoperation& aoperation::operator=(aoperation && other) {
    return swap(other);
}

aoperation::~aoperation() {
    if (valid) {
        e->unsubscribe(fd, event);
    }
}

aoperation& aoperation::swap(aoperation& other) {
    std::swap(valid, other.valid);
    std::swap(e, other.e);
    std::swap(event, other.event);
    std::swap(fd, other.fd);
    return *this;
}
