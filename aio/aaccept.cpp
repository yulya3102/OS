#include "aaccept.h"
#include "epollfd.h"
#include "aoperation.h"
#include <sys/socket.h>
#include <sys/types.h>

aaccept::aaccept(epollfd& e, int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int, struct sockaddr *, socklen_t *)> cont_ok, std::function<void()> cont_err)
    : aoperation(e, EPOLLIN, fd) {
    std::function<void()> cont = [this, fd, addr, addrlen, cont_ok] () {
        valid = false;
        int acceptedfd = accept(fd, addr, addrlen);
        cont_ok(acceptedfd, addr, addrlen);
    };
    e.subscribe(fd, event, cont, cont_err);
}
