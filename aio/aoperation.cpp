#include "aoperation.h"
#include "epollfd.h"

aoperation::aoperation(epollfd& e, int event, int fd)
    : valid(true)
    , e(&e)
    , event(event)
    , fd(fd)
{}

aoperation::~aoperation() {
    if (valid) {
        e->unsubscribe(fd, event);
    }
}
