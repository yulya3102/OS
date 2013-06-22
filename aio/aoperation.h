#pragma once

#include "epollfd.h"

struct aoperation {
    aoperation(aoperation const&) = delete;
    aoperation& operator=(aoperation const&) = delete;
    ~aoperation();
protected:
    aoperation(epollfd& e, int event, int fd);
    bool valid;
    epollfd * e;
    int event;
    int fd;
};
