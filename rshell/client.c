#include "common.h"

#include <sys/ioctl.h>
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
#include <termios.h>

#define UNUSED(x) (void)(x)

int window_size_changed = 1;

void sigwinch_handler(int signum) {
    UNUSED(signum);
    window_size_changed = 1;
}

typedef struct winsize winsize_t;

winsize_t get_window_size(int fd) {
    winsize_t ws;
    check("ioctl", ioctl(fd, TIOCGWINSZ, &ws));
    return ws;
}

int main() {
    int ttyfd = check("open", open("/dev/tty", O_RDONLY));
    signal(SIGWINCH, sigwinch_handler);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo * result;
    if (getaddrinfo(0, "8822", &hints, &result) != 0) {
        printf("getaddrinfo() failed\n");
        _exit(1);
    }
    int socketfd = check("socket", socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    check("connect", connect(socketfd, result->ai_addr, result->ai_addrlen));

    int buffer_size = 4096;

    short error_events = POLLERR | POLLHUP | POLLNVAL;

    int pollfds_size = 3;
    struct pollfd pollfds[3];

    struct pollfd input_pollfd;
    input_pollfd.fd = 0;
    input_pollfd.events = POLLIN | error_events;
    pollfds[0] = input_pollfd;
    buffer_t inputb = new_buffer(buffer_size);
    int input_eof = 0;

    struct pollfd socket_pollfd;
    socket_pollfd.fd = socketfd;
    socket_pollfd.events = POLLIN | error_events;
    pollfds[1] = socket_pollfd;
    buffer_t socketb = new_buffer(buffer_size);
    int socket_eof = 0;

    struct pollfd output_pollfd;
    output_pollfd.fd = 1;
    output_pollfd.events = error_events;
    pollfds[2] = output_pollfd;
    
    while (!input_eof || !socket_eof || inputb.current_size > 0 || socketb.current_size > 0) {
        check("poll", poll(pollfds, pollfds_size, -1));
        if (pollfds[0].revents & POLLIN) {
            inputb.buffer[inputb.current_size] = 't';
            inputb.current_size++;
            int r = check("read", read(0, inputb.buffer + inputb.current_size, inputb.size - 1 - inputb.current_size));
            if (r == 0) {
                input_eof = 1;
            } else {
                inputb.current_size += r;
            }
            inputb.buffer[inputb.current_size] = '\0';
            inputb.current_size++;
        }
        if (pollfds[1].revents & POLLIN) {
            int r = check("read", read(socketfd, socketb.buffer + socketb.current_size, buffer_size - socketb.current_size));
            if (r == 0) {
                socket_eof = 1;
            } else {
                socketb.current_size += r;
            }
        }
        if (pollfds[1].revents & POLLOUT) {
            int r = check("write", write(socketfd, inputb.buffer, inputb.current_size));
            memmove(inputb.buffer, inputb.buffer + r, inputb.current_size - r);
            inputb.current_size -= r;
        }
        if (pollfds[2].revents & POLLOUT) {
            int r = check("write", write(1, socketb.buffer, socketb.current_size));
            memmove(socketb.buffer, socketb.buffer + r, socketb.current_size - r);
            socketb.current_size -= r;
        }

        if (window_size_changed) {
            winsize_t ws = get_window_size(ttyfd);
            if (buffer_size > inputb.current_size + 6) {
                inputb.buffer[inputb.current_size] = 'r';
                inputb.current_size++;
                *(short *)(inputb.buffer + inputb.current_size) = ws.ws_row;
                inputb.current_size += 2;
                *(short *)(inputb.buffer + inputb.current_size) = ws.ws_col;
                inputb.current_size += 2;
                inputb.buffer[inputb.current_size] = '\0';
                inputb.current_size++;
                window_size_changed = 0;
            }
        }

        if (pollfds[0].revents & error_events) {
            input_eof = 1;
        }
        if (pollfds[1].revents & error_events) {
            socket_eof = 1;
            inputb.current_size = 0;
            input_eof = 1;
        }
        if (pollfds[2].revents & error_events) {
            socketb.current_size = 0;
            socket_eof = 1;
            inputb.current_size = 0;
            input_eof = 1;
        }

        // update events
        if (input_eof || inputb.current_size == buffer_size) {
            pollfds[0].events &= ~POLLIN;
        }
        if (inputb.current_size == 0) {
            pollfds[1].events &= ~POLLOUT;
        }
        if (inputb.current_size > 0 && inputb.current_size < buffer_size) {
            pollfds[1].events |= POLLOUT;
        }
        if (inputb.current_size < buffer_size - 2 && !input_eof) {
            pollfds[0].events |= POLLIN;
        }
        if (socket_eof || socketb.current_size == buffer_size) {
            pollfds[1].events &= ~POLLIN;
        }
        if (socketb.current_size == 0) {
            pollfds[2].events &= ~POLLOUT;
        } else if (socketb.current_size < buffer_size) {
            pollfds[2].events |= POLLOUT;
            if (!socket_eof) {
                pollfds[1].events |= POLLIN;
            }
        }
    }
    return 0;
}
