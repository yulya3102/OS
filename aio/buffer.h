#pragma once

struct buffer {
    buffer(int max_size);
    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;
    void read(int fd);
    void write(int fd);
    ~buffer();

private:
    char * buf;
    int max_size, current_size;
};
