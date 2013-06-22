#include "epollfd.h"
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

#define UNUSED(x) (void)(x)

static const int MAXSIZE = 10;
static const int ERROR_EVENTS = EPOLLERR;

static int check(const char * message, int result) {
    if (result == -1) {
        throw std::runtime_error(std::string(message) + std::string(": ") + std::string(strerror(errno)));
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
        epoll_events[fd].events = what;
        epoll_events[fd].data.fd = fd;
        check("epoll_ctl", epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_events[fd]));
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
    conts_err[fd].erase(conts_err[fd].find(what));
}

void epollfd::cycle() {
    struct epoll_event events[MAXSIZE];
    int n = epoll_wait(epoll_fd, events, MAXSIZE, -1);
    if (n == -1) {
        if (errno == EINTR) {
            return;
        }
        throw std::runtime_error("epoll_wait failed: " + std::string(strerror(errno)));
    }
    for (int i = 0; i < n; i++) {
        struct epoll_event ev = events[i];
        int fd = ev.data.fd;
        if (ev.events & ERROR_EVENTS) {
            for (auto it = conts_err[fd].begin(); it != conts_err[fd].end(); ) {
                int event = (*it).first;
                auto cont = (*it).second;
                it++;
                unsubscribe(fd, event);
                cont();
            }
            for (auto it = conts[fd].begin(); it != conts[fd].end(); ) {
                int event = it->first;
                it++;
                unsubscribe(fd, event);
            }
        } else {
            for (auto it = conts[fd].begin(); it != conts[fd].end(); ) {
                int event = (*it).first;
                auto cont = (*it).second;
                it++;
                if (event & ev.events) {
                    unsubscribe(fd, event);
                    cont();
                }
            }
        }
    }
}

epollfd::~epollfd() {
    close(epoll_fd);
}
