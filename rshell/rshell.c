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

int daemon_pid;

void sigint_handler(int signum) {
    kill(daemon_pid, SIGINT);
    _exit(1);
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
                int master_buffer_size = 4096;
                char * master_buffer = malloc(master_buffer_size);
                if (master_buffer == NULL) {
                    char * message = "malloc() failed";
                    write_(1, message, strlen(message));
                    _exit(1);
                }
                int master_buffer_full = 0;
                int accepted_buffer_size = 4096;
                char * accepted_buffer = malloc(accepted_buffer_size);
                if (accepted_buffer == NULL) {
                    char * message = "malloc() failed";
                    write_(1, message, strlen(message));
                    _exit(1);
                }
                int accepted_buffer_full = 0;
                int pollfds_size = 2;
                struct pollfd * pollfds = malloc(sizeof(struct pollfd *) * pollfds_size); // TODO: check
                struct pollfd masterpollfd;
                masterpollfd.fd = master;
                masterpollfd.events = POLLIN;
                masterpollfd.revents = 0;
                struct pollfd acceptedpollfd;
                acceptedpollfd.fd = accepted;
                acceptedpollfd.events = POLLIN;
                acceptedpollfd.revents = 0;
                pollfds[0] = masterpollfd;
                pollfds[1] = acceptedpollfd;
                int master_eof = 0;
                int accepted_eof = 0;
                while (!master_eof || !accepted_eof) {
                    int k = check("poll", poll(pollfds, pollfds_size, -1));
                    if (pollfds[0].revents & POLLIN) {
                        int r = check("read", read(master, master_buffer + master_buffer_full, master_buffer_size - master_buffer_full));
                        if (r == 0) {
                            master_eof = 1;
                            pollfds[0].events = 0;
                            close_(master);
                        } else {
                            master_buffer_full += r;
                            pollfds[1].events |= POLLOUT;
                            if (master_buffer_full == master_buffer_size) {
                                pollfds[0].events ^= POLLIN;
                            }
                        }
                    }
                    if (pollfds[1].revents & POLLOUT) {
                        write_(accepted, master_buffer, master_buffer_full);
                        master_buffer_full = 0;
                        pollfds[1].events ^= POLLOUT;
                        pollfds[0].events |= POLLIN;
                    }
                    if (pollfds[1].revents & POLLIN) {
                        int r = check("read", read(accepted, accepted_buffer + accepted_buffer_full, accepted_buffer_size - accepted_buffer_full));
                        if (r == 0) {
                            accepted_eof = 1;
                            pollfds[1].events = 0;
                            close_(accepted);
                        } else {
                            accepted_buffer_full += r;
                            pollfds[0].events |= POLLOUT;
                            if (accepted_buffer_full == accepted_buffer_size) {
                                pollfds[1].events ^= POLLIN;
                            }
                        }
                    }
                    if (pollfds[0].revents & POLLOUT) {
                        write_(master, accepted_buffer, accepted_buffer_full);
                        accepted_buffer_full = 0;
                        pollfds[0].events ^= POLLOUT;
                        pollfds[1].events |= POLLIN;
                    }
                }
                free(master_buffer);
                free(accepted_buffer);
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
