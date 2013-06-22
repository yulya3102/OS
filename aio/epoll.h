#pragma once

#include "buffer.h"
#include "aoperation.h"
#include "epollfd.h"

#include <functional>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>

struct epoll {
    epoll();
    epoll(epoll const& other) = delete;
    epoll& operator=(epoll const& other) = delete;
    void read(int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err);
    void write(int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err);
    void accept(int fd, struct sockaddr * addr, socklen_t * addrlen, std::function<void(int)> cont_ok, std::function<void()> cont_err);
    void cycle();
    ~epoll();

private:
    std::list<aoperation*> operations;
    epollfd e;
};
