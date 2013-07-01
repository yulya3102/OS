#pragma once

#include "epoll.h"

#include <functional>
#include <stdexcept>

struct copy {
    copy(int fromfd, int tofd, int speed, int maxspeed);
    copy(const copy&) = delete;
    copy(copy &&) = delete;
    copy& operator=(const copy&) = delete;
    copy& operator=(copy &&) = delete;
    ~copy() = default;

private:
    int timerfd;
    epoll e;
    std::function<void()> error_cont;
};
