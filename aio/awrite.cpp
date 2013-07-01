#include "awrite.h"
#include "epollfd.h"
#include "buffer.h"

#include <functional>

awrite::awrite(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup)
    : aoperation(e, EPOLLOUT, fd) {
    auto econt = [&buf, fd, this, cont_ok] () {
        buf.write(fd);
        valid = false;
        cont_ok();
    };
    e.subscribe(fd, event, econt, cont_err, cont_epollhup);
}
