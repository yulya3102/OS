#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

#define UNUSED(x) (void)(x)

int window_size_changed = 0;

void sigwinch_handler(int signum) {
    UNUSED(signum);
    window_size_changed = 1;
}

int check(char * message, int value) {
    if (value == -1) {
        perror(message);
        _exit(1);
    }
    return value;
}

int main() {
    signal(SIGWINCH, sigwinch_handler);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo * result;
    getaddrinfo(0, "8822", &hints, &result);
    int socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    connect(socketfd, result->ai_addr, result->ai_addrlen);

    int buffer_size = 4096;

    short error_events = POLLERR | POLLHUP | POLLNVAL;

    int pollfds_size = 3;
    struct pollfd * pollfds = malloc(sizeof(struct pollfd) * pollfds_size);

    struct pollfd input_pollfd;
    input_pollfd.fd = 0;
    input_pollfd.events = POLLIN | error_events;
    pollfds[0] = input_pollfd;
    char * input_buffer = malloc(buffer_size);
    int input_size = 0;
    int input_eof = 0;

    struct pollfd socket_pollfd;
    socket_pollfd.fd = socketfd;
    socket_pollfd.events = POLLIN | error_events;
    pollfds[1] = socket_pollfd;
    char * socket_buffer = malloc(buffer_size);
    int socket_size = 0;
    int socket_eof = 0;

    struct pollfd output_pollfd;
    output_pollfd.fd = 1;
    output_pollfd.events = error_events;
    pollfds[2] = output_pollfd;
    
    while (!input_eof || !socket_eof || input_size > 0 || socket_size > 0) {
        poll(pollfds, pollfds_size, -1);
        if (pollfds[0].revents & POLLIN) {
            input_buffer[input_size] = 't';
            input_size++;
            int r = check("read", read(0, input_buffer + input_size, buffer_size - 1 - input_size));
            if (r == 0) {
                input_eof = 1;
            } else {
                input_size += r;
            }
            input_buffer[input_size] = '\0';
            input_size++;
        }
        if (pollfds[1].revents & POLLIN) {
            int r = check("read", read(socketfd, socket_buffer + socket_size, buffer_size - socket_size));
            if (r == 0) {
                socket_eof = 1;
            } else {
                socket_size += r;
            }
        }
        if (pollfds[1].revents & POLLOUT) {
            int r = check("write", write(socketfd, input_buffer, input_size));
            memmove(input_buffer, input_buffer + r, input_size - r);
            input_size -= r;
        }
        if (pollfds[2].revents & POLLOUT) {
            int r = check("write", write(1, socket_buffer, socket_size));
            memmove(socket_buffer, socket_buffer + r, socket_size - r);
            socket_size -= r;
        }

        // update events
        if (input_eof || input_size == buffer_size) {
            pollfds[0].events &= ~POLLIN;
        }
        if (input_size == 0) {
            pollfds[1].events &= ~POLLOUT;
        } else if (input_size < buffer_size - 2) {
            pollfds[1].events |= POLLOUT;
            if (!input_eof) {
                pollfds[0].events |= POLLIN;
            }
        }
        if (socket_eof || socket_size == buffer_size) {
            pollfds[1].events &= ~POLLIN;
        }
        if (socket_size == 0) {
            pollfds[2].events &= ~POLLOUT;
        } else if (socket_size < buffer_size) {
            pollfds[2].events |= POLLOUT;
            if (!socket_eof) {
                pollfds[1].events |= POLLIN;
            }
        }

        if (window_size_changed) {
            char * message = "new window size";
            write(1, message, strlen(message));
            window_size_changed = 0;
        }
    }
    return 0;
}
