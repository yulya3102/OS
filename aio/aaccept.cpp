#include "aaccept.h"
#include "epollfd.h"
#include "aoperation.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdexcept>
#include <cstring>

aaccept::aaccept(epollfd& e, int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int)> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup)
    : aoperation(e, EPOLLIN, fd) {
    std::function<void()> cont = [this, fd, addr, addrlen, cont_ok] () {
        valid = false;
        int acceptedfd = accept(fd, addr, addrlen);
        if (acceptedfd == -1) {
            throw std::runtime_error(std::string("accept: ") + std::string(strerror(errno)));
        }
        cont_ok(acceptedfd);
    };
    e.subscribe(fd, event, cont, cont_err, cont_epollhup);
}
