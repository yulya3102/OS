#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define LISTEN_BACKLOG 50

int main(int argc, char ** argv) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo * result;
    if (getaddrinfo(0, "8822", &hints, &result) == 0) {
        printf("getaddrinfo()\n");
    } else {
        printf("getaddrinfo() failed\n");
        _exit(1);
    }
    int socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socketfd == -1) {
        perror("socket() failed");
        _exit(1);
    } else {
        printf("socket fd: %i\n", socketfd);
    }
    int option = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option)) == 0) {
        printf("setsockopt()\n");
    } else {
        perror("setsockopt() failed");
        _exit(1);
    }
    if (bind(socketfd, result->ai_addr, result->ai_addrlen) == 0) {
        printf("bind()\n");
    } else {
        perror("bind() failed");
        _exit(1);
    }
    if (listen(socketfd, LISTEN_BACKLOG) == 0) {
        printf("listen()\n");
    } else {
        perror("listen() failed");
        _exit(1);
    }
    struct sockaddr peer_addr;
    socklen_t peer_addr_size;
    int acceptedfd = accept(socketfd, &peer_addr, &peer_addr_size);
    if (acceptedfd == -1) {
        perror("accept() failed");
        _exit(1);
    } else {
        printf("accepted fd: %i\n", socketfd);
    }

    if (fork()) {
        if (close(acceptedfd) == -1) {
            perror("close failed");
            _exit(1);
        }
    } else {
        dup2(acceptedfd, 0);
        dup2(acceptedfd, 1);
        dup2(acceptedfd, 2);
        if (close(acceptedfd) == -1) {
            perror("close failed");
            _exit(1);
        }
        printf("hello, world\n");
    }
}
