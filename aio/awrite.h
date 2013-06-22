#pragma once

#include "aoperation.h"
#include "epollfd.h"
#include "buffer.h"

struct awrite : aoperation {
    awrite(epollfd& e, int fd, buffer& buf, std::function<void()> cont_ok, std::function<void()> cont_err);
};
