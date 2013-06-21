#pragma once

#include <functional>
#include <sys/epoll.h>
#include <map>

struct epollfd {
    epollfd();
    epollfd(epollfd const&) = delete;
    epollfd& operator=(epollfd const& other) = delete;
    void subscribe(int fd, int what, std::function<void()> cont_ok, std::function<void()> cont_err);
    void unsubscribe(int fd, int what);
    void cycle();
    ~epollfd();

private:
    int epoll_fd;
    std::map<int, std::map<int, std::function<void()> > > conts;
    std::map<int, std::map<int, std::function<void()> > > conts_err;
    std::map<int, struct epoll_event> epoll_events;
};
