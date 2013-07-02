#pragma once

struct autofd {
    autofd(int fd);
    autofd(autofd const&) = delete;
    autofd(autofd &&);
    autofd& operator=(autofd const&) = delete;
    autofd& operator=(autofd &&);
    const int& operator*() const;
    ~autofd();
private:
    int fd;
};
