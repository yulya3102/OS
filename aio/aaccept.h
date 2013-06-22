#pragma once

#include "epollfd.h"
#include "aoperation.h"

#include <sys/types.h>
#include <sys/socket.h>

struct aaccept : aoperation {
    aaccept(epollfd& e, int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int, struct sockaddr *, socklen_t *)> cont_ok, std::function<void()> cont_err);
};
