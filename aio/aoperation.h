#pragma once

#include "epollfd.h"

struct aoperation {
    aoperation(aoperation const&) = delete;
    aoperation(aoperation && other);
    aoperation& operator=(aoperation const&) = delete;
    aoperation& operator=(aoperation && other);
    ~aoperation();
protected:
    aoperation(epollfd& e, int event, int fd);
    bool valid;
    epollfd * e;
    int event;
    int fd;
private:
    aoperation& swap(aoperation& other);
};
