#include "aread.h"

aread::aread(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err)
    : valid(true)
    , e(&e)
    , fd(fd) {
    auto epollfd_cont_ok = [this, fd, &buf, &cont_ok] () {
        buf.read(fd);
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
