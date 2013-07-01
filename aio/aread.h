#pragma once

#include "epollfd.h"
#include "buffer.h"
#include "aoperation.h"

struct aread : aoperation {
    aread(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup);
    aread(epollfd& e, int fd, buffer& buf, int limit, std::function<void()> cont_ok, std::function<void()> cont_err, std::function<void()> cont_epollhup);
};
