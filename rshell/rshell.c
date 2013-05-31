#include <pty.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <poll.h>

#define LISTEN_BACKLOG 50

int check(const char * message, int result) {
    if (result == -1) {
        perror(message);
        _exit(1);
    }
    return result;
}

void write_(int fd, char * buffer, int size) {
    int done = 0;
    while (done < size) {
        done += check("write", write(fd, buffer + done, size - done));
    }
}

void close_(int fd) {
    check("close", close(fd));
}

void dup2_(int fd1, int fd2) {
    check("dup2", dup2(fd1, fd2));
}

void * malloc_(int size) {
    void * buffer = malloc(size);
    if (buffer == NULL) {
        char * message = "malloc() failed";
        write_(1, message, strlen(message));
        _exit(1);
    }
    return buffer;
}

int daemon_pid;

void sigint_handler(int signum) {
    kill(daemon_pid, SIGINT);
    _exit(1);
}

void read_(int fd, char * buffer, int max_size, int * current_size, int * eof) {
    int r = check("read", read(fd, buffer + *current_size, max_size - *current_size));
    if (r == 0) {
        *eof = 1;
    } else {
        *current_size += r;
    }
}

void update_events(int eof, int current_size, int max_size, struct pollfd * in, struct pollfd * out) {
    if (eof || current_size == max_size) {
        in->events &= ~POLLIN;
    }
    if (current_size == 0) {
        out->events &= ~POLLOUT;
    } else if (current_size < max_size) {
        out->events |= POLLOUT;
        if (!eof) {
            in->events |= POLLIN;
        }
    }
}

typedef struct {
    int max_size;
    int current_size;
    char * buffer;
    int eof;
    int fd;
} buffer_t;

buffer_t new_buffer(int max_size, int fd) {
    buffer_t buffer;
    buffer.max_size = max_size;
    buffer.current_size = 0;
    buffer.buffer = malloc_(max_size);
    buffer.eof = 0;
    buffer.fd = fd;
    return buffer;
}

int main() {
    daemon_pid = check("fork", fork());
    signal(SIGINT, &sigint_handler);
    if (daemon_pid) {
        int status;
        waitpid(daemon_pid, &status, 0);
        return 0;
    }
    check("setsid", setsid());
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
    int option = 1;
    check("setsockopt", setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option)));
    check("bind", bind(socketfd, result->ai_addr, result->ai_addrlen));
    check("listen", listen(socketfd, LISTEN_BACKLOG));
    while (1) {
        printf("waiting for connection\n");
        int accepted = check("accept", accept(socketfd, NULL, NULL));
        int acceptedpid = check("fork", fork());
        if (acceptedpid) {
            close_(accepted);
        } else { // прокся
            close_(socketfd);
            char * hello = "hello, world\n";
            write_(accepted, hello, strlen(hello));
            int master, slave;
            char buf[4096];
            check("openpty", openpty(&master, &slave, buf, NULL, NULL));
            if (check("fork", fork())) {
                close_(slave);

                int size = 2;

                buffer_t * buffers = malloc_(sizeof(buffer_t) * size);
                buffers[0] = new_buffer(4096, master);
                buffers[1] = new_buffer(4096, accepted);

                short error_events = POLLERR | POLLHUP | POLLNVAL;
                struct pollfd * pollfds = malloc_(sizeof(struct pollfd *) * size);
                int i;
                for (i = 0; i < size; i++) {
                    struct pollfd pfd;
                    pfd.fd = buffers[i].fd;
                    pfd.events = POLLIN | error_events;
                    pfd.revents = 0;
                    pollfds[i] = pfd;
                }
                while (!buffers[0].eof || !buffers[1].eof || buffers[0].current_size > 0 || buffers[0].current_size > 0) {
                    int k = check("poll", poll(pollfds, size, -1));
                    for (i = 0; i < size; i++) {
                        if (pollfds[i].revents & POLLIN) {
                            read_(buffers[i].fd, buffers[i].buffer, buffers[i].max_size, &buffers[i].current_size, &buffers[i].eof);
                        }
                        if (pollfds[i].revents & POLLOUT) {
                            int r = check("write", write(buffers[i].fd, buffers[1 - i].buffer, buffers[1 - i].current_size));
                            memmove(buffers[1 - i].buffer, buffers[1 - i].buffer + r, buffers[1 - i].current_size - r);
                            buffers[1 - i].current_size -= r;
                        }
                        if (pollfds[i].revents & error_events) {
                            buffers[i].eof = 1;
                            buffers[1 - i].current_size = 0;
                            buffers[1 - i].eof = 1;
                        }
                    }

                    for (i = 0; i < size; i++) {
                        update_events(buffers[i].eof, buffers[i].current_size, buffers[i].max_size, &pollfds[i], &pollfds[1 - i]);
                    }
                }
                for (i = 0; i < size; i++) {
                    close(buffers[i].fd);
                    free(buffers[i].buffer);
                }
                free(buffers);
            } else { // сессия
                close_(master);
                close_(accepted);
                check("setsid", setsid());
                int fd = check("open", open(buf, O_RDWR));
                close_(fd);

                dup2_(slave, 0);
                dup2_(slave, 1);
                dup2_(slave, 2);
                close_(slave);
                execl("/bin/sh", "/bin/sh", NULL);
            }
        }
    }
    return 0;
}
