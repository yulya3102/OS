#include "epollfd.h"
#include "buffer.h"
#include "aread.h"
#include "awrite.h"
#include "aaccept.h"
#include "epoll.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define LISTEN_BACKLOG 10
#define UNUSED(x) (void)(x)

int main() {
    auto error_action = [] () { std::cout << "error" << std::endl; };
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo * result;
    if (getaddrinfo(0, "8822", &hints, &result) != 0) {
        error_action();
        _exit(1);
    }
    int socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    int option = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option));
    bind(socketfd, result->ai_addr, result->ai_addrlen);
    free(result);
    result = nullptr;
    listen(socketfd, LISTEN_BACKLOG);
    printf("waiting for connection\n");
    epoll e;
    auto cont = [&e, error_action] (int fd) {
        buffer buf(4096);
        auto read_cont = [&e, fd, &buf, error_action] () {
            e.write(fd, buf, [] () {}, error_action);
            e.cycle();
        };
        e.read(0, buf, read_cont, error_action);
        e.cycle();
    };
    e.accept(socketfd, nullptr, nullptr, cont, error_action);
    e.cycle();
}
