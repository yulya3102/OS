#include "epollfd.h"
#include "buffer.h"
#include "aread.h"
#include "awrite.h"
#include "aaccept.h"

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
    epollfd fd;
    std::function<void(int, struct sockaddr*, socklen_t*)> acceptcont = [&fd, error_action] (int acceptedfd, struct sockaddr * addr, socklen_t * addrlen) {
        UNUSED(addr);
        UNUSED(addrlen);
        buffer buf(4096);
        aread ar(fd, 0, buf, [] () {}, error_action);
        fd.cycle();
        awrite aw(fd, acceptedfd, buf, [] () {}, error_action);
        fd.cycle();
    };
    aaccept aa(fd, socketfd, NULL, NULL, acceptcont, error_action);
    fd.cycle();
}
