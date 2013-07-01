#pragma once

#include "var.h"

struct buffer {
    buffer(int max_size);
    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;
    void read(int fd);
    void write(int fd);
    var<int>& size();
    ~buffer();

    const int max_size;

private:
    char * buf;
    var<int> current_size;
};
