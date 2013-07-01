#include "aread.h"

aread::aread(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup)
    : aoperation(e, EPOLLIN, fd) {
    auto epollfd_cont_ok = [this, fd, &buf, cont_ok] () {
        buf.read(fd);
        valid = false;
        cont_ok();
    };
    e.subscribe(fd, event, epollfd_cont_ok, cont_err, cont_epollhup);
}
