#include "aread.h"

#include <iostream>

aread::aread(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err)
    : valid(true)
    , e(&e)
    , fd(fd) {
    auto epollfd_cont_ok = [this, &buf, &cont_ok] () {
        buf.read();
        valid = false;
        cont_ok();
    };
    e.subscribe(fd, EPOLLIN, epollfd_cont_ok, cont_err);
}

aread::~aread() {
    if (valid) {
        e->unsubscribe(fd, EPOLLIN);
    }
}
