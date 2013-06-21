#include "epollfd.h"
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

static const int MAXSIZE = 10;
static const int ERROR_EVENTS = EPOLLERR;

static int check(const char * message, int result) {
    if (result == -1) {
        throw std::runtime_error(message);
    }
    return result;
}

epollfd::epollfd()
    : epoll_fd(check("epoll_create", epoll_create(MAXSIZE)))
    {}

void epollfd::subscribe(int fd, int what, std::function<void()> cont_ok, std::function<void()> cont_err) {
    if (conts.find(fd) != conts.end() && conts[fd].find(what) != conts[fd].end()) {
        throw std::runtime_error("this fd already has this event");
    }
    if (epoll_events.find(fd) == epoll_events.end()) {
        struct epoll_event ev;
        ev.events = what;
        ev.data.fd = fd;
        check("epoll_ctl", epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev));
        epoll_events[fd] = ev;
    } else {
        struct epoll_event ev = epoll_events[fd];
        ev.events |= what;
        check("epoll_ctl", epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev));
    }
    conts[fd][what] = cont_ok;
    conts_err[fd][what] = cont_err;
}

void epollfd::unsubscribe(int fd, int what) {
    if (conts.find(fd) == conts.end() || conts[fd].find(what) == conts[fd].end()) {
        throw std::runtime_error("this fd doesn't have this event");
    }
    struct epoll_event ev = epoll_events[fd];
    ev.events |= ~what;
    check("epoll_ctl", epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev));
    conts[fd].erase(conts[fd].find(what));
}

void epollfd::cycle() {
    struct epoll_event events[MAXSIZE];
    int n = check("epoll_wait", epoll_wait(epoll_fd, events, MAXSIZE, -1));
    for (int i = 0; i < n; i++) {
        struct epoll_event ev = events[i];
        int fd = ev.data.fd;
        if (ev.events & ERROR_EVENTS) {
            for (auto it = conts_err[fd].begin(); it != conts_err[fd].end(); it++) {
                int event = (*it).first;
                auto cont = (*it).second;
                unsubscribe(fd, event);
                cont();
            }
            for (auto it = conts[fd].begin(); it != conts[fd].end(); it++) {
                unsubscribe(fd, (*it).first);
            }
        } else {
            for (auto it = conts[fd].begin(); it != conts[fd].end(); it++) {
                int event = (*it).first;
                auto cont = (*it).second;
                unsubscribe(fd, event);
                cont();
            }
        }
    }
}

epollfd::~epollfd() {
    close(epoll_fd);
}
