#include "aaccept.h"
#include "epollfd.h"
#include <sys/socket.h>
#include <sys/types.h>

aaccept::aaccept(epollfd& e, int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int, struct sockaddr *, socklen_t *)> cont_ok, std::function<void()> cont_err)
    : valid(true)
    , e(&e)
    , fd(fd) {
    std::function<void()> cont = [this, fd, addr, addrlen, cont_ok] () {
        valid = false;
        int acceptedfd = accept(fd, addr, addrlen);
        cont_ok(acceptedfd, addr, addrlen);
    };
    e.subscribe(fd, EPOLLIN, cont, cont_err);
}

aaccept::~aaccept() {
    if (valid) {
        e->unsubscribe(fd, EPOLLIN);
    }
}
