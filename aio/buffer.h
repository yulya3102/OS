#pragma once

struct buffer {
    buffer(int fd, int max_size);
    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;
    void read();

private:
    char * buf;
    int max_size, current_size;
    int fd;
    bool eof;
};
