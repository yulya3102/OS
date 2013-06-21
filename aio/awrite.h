#pragma once

#include "epollfd.h"
#include "buffer.h"

struct awrite {
    awrite(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err);
    awrite(awrite const&) = delete;
    awrite& operator=(awrite const&) = delete;
    ~awrite();

private:
    epollfd * e;
    bool valid;
    int fd;
};
