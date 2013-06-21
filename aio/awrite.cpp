#include "awrite.h"

awrite::awrite(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err)
    : e(&e)
    , valid(true)
    , fd(fd) {
    auto econt = [&buf, fd, this, &cont_ok] () {
        buf.write(fd);
        valid = false;
        cont_ok();
    };
    e.subscribe(fd, EPOLLOUT, econt, cont_err);
}

awrite::~awrite() {
    if (valid) {
        e->unsubscribe(fd, EPOLLOUT);
    }
}
