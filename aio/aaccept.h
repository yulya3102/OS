#pragma once

#include "epollfd.h"

#include <sys/types.h>
#include <sys/socket.h>

struct aaccept {
    aaccept(epollfd& e, int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int, struct sockaddr *, socklen_t *)> cont_ok, std::function<void()> cont_err);
    aaccept(aaccept const&) = delete;
    aaccept& operator=(aaccept const&) = delete;
    ~aaccept();

private:
    bool valid;
    epollfd * e;
    int fd;
};
