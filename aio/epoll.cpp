#include "epoll.h"
#include "aread.h"
#include "awrite.h"
#include "aaccept.h"
#include "buffer.h"

#include <functional>
#include <sys/types.h>
#include <sys/socket.h>

epoll::epoll() {}

void epoll::read(int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup) {
    operations.push_back(nullptr);
    aoperation *& ar = operations.back();
    auto cont = [this, &ar, cont_ok] () {
        delete ar;
        ar = nullptr;
        operations.remove(ar);
        cont_ok();
    };
    ar = new aread(e, fd, buf, cont, cont_err, cont_epollhup);
}

void epoll::write(int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup) {
    operations.push_back(nullptr);
    aoperation *& aw = operations.back();
    auto cont = [this, &aw, cont_ok] () {
        delete aw;
        aw = nullptr;
        operations.remove(aw);
        cont_ok();
    };
    aw = new awrite(e, fd, buf, cont, cont_err, cont_epollhup);
}

void epoll::accept(int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int)> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup) {
    operations.push_back(nullptr);
    aoperation *& aa = operations.back();
    auto cont = [this, &aa, cont_ok] (int fd) {
        delete aa;
        aa = nullptr;
        operations.remove(aa);
        cont_ok(fd);
    };
    aa = new aaccept(e, fd, addr, addrlen, cont, cont_err, cont_epollhup);
}

void epoll::cycle() {
    e.cycle();
}

epoll::~epoll() {
    for (auto it = operations.begin(); it != operations.end();) {
        aoperation * aoperation = *it;
        ++it;
        delete aoperation;
    }
}
