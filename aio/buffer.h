#pragma once

#include "var.h"

struct buffer {
    buffer(int max_size);
    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;
    void read(int fd);
    void read(int fd, int size);
    void write(int fd);
    void write(void * dest, int size);
    var<int>& size();
    ~buffer();

    const int max_size;

private:
    char * buf;
    var<int> current_size;
};
