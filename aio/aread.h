#pragma once

#include "epollfd.h"
#include "buffer.h"

struct aread {
    aread(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err);
    aread(aread const&) = delete;
    aread& operator=(aread const&) = delete;
    ~aread();

private:
    bool valid;
    epollfd * e;
    int fd;
};
