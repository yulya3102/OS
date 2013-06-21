#include "epollfd.h"
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

static const int MAXSIZE = 10;

epollfd::epollfd()
    : epoll_fd(epoll_create(MAXSIZE))
    {}

void epollfd::subscribe(int fd, int what, std::function<void()> cont_ok, std::function<void()> cont_err) {
    // TODO: fix this
    UNUSED(cont_err);
    if (conts.find(fd) != conts.end() && conts[fd].find(what) != conts[fd].end()) {
        throw std::runtime_error("this fd already has this event");
    }
    if (epoll_events.find(fd) == epoll_events.end()) {
        struct epoll_event ev;
        ev.events = what;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        epoll_events[fd] = ev;
        conts[fd][what] = cont_ok;
    } else {
        struct epoll_event ev = epoll_events[fd];
        ev.events |= what;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
        conts[fd][what] = cont_ok;
    }
}

void epollfd::unsubscribe(int fd, int what) {
    if (conts.find(fd) == conts.end() || conts[fd].find(what) == conts[fd].end()) {
        throw std::runtime_error("this fd doesn't have this event");
    }
    struct epoll_event ev = epoll_events[fd];
    ev.events |= ~what;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    conts[fd].erase(conts[fd].find(what));
}

void epollfd::cycle() {
    struct epoll_event events[MAXSIZE];
    int n = epoll_wait(epoll_fd, events, MAXSIZE, -1);
    // TODO: check for errors
    for (int i = 0; i < n; i++) {
        struct epoll_event ev = events[i];
        int fd = ev.data.fd;
        for (int event = 1; event != 0; event <<= 1) {
            if (ev.events & event) {
                auto cont = conts[fd][event];
                unsubscribe(fd, event);
                cont();
            }
        }
    }
}

epollfd::~epollfd() {
    close(epoll_fd);
}
