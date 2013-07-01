#pragma once

struct autofd {
    autofd(int fd);
    autofd(autofd const&) = delete;
    autofd(autofd &&);
    autofd& operator=(autofd const&) = delete;
    autofd& operator=(autofd &&);
    ~autofd();
private:
    int fd;
};
