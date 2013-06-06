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

#define UNUSED(x) (void)(x)

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
    UNUSED(signum);
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
    int size, current_size;
    char * buffer;
} buffer_t;

buffer_t new_buffer(int size) {
    buffer_t buffer;
    buffer.size = size;
    buffer.current_size = 0;
    buffer.buffer = malloc_(size);
    return buffer;
}

int pos(char symbol, char * buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (buffer[i] == symbol) {
            return i;
        }
    }
    return -1;
}

void set_terminal_size(int ttyfd, short rows, short columns) {
    struct winsize ws;
    ws.ws_row = rows;
    ws.ws_col = columns;
    check("ioctl", ioctl(ttyfd, TIOCSWINSZ, &ws));
}

void preprocess_data(int ttyfd, buffer_t * accepted, buffer_t * preprocessed, int * accepted_state) {
    if (*accepted_state == 0) { // new event
        if (accepted->buffer[0] == 'r') {
            *accepted_state = 1;
        } else if (accepted->buffer[0] == 't') {
            *accepted_state = 2;
        } else {
            char * message = "unknown event\n";
            write(1, message, strlen(message));
            _exit(1);
        }
        accepted->current_size--;
        memmove(accepted->buffer, accepted->buffer + 1, accepted->current_size);
    }
    if (*accepted_state == 1) { // resize event
        short rows = *((short *)(accepted->buffer));
        short cols = *((short *)(accepted->buffer + 2));
        set_terminal_size(ttyfd, rows, cols);
        int deleted_string_length = 5;
        memmove(accepted->buffer, accepted->buffer + deleted_string_length, accepted->current_size - deleted_string_length);
        accepted->current_size -= deleted_string_length;
        *accepted_state = 0;
    } else if (*accepted_state == 2) { // type event
        int p = pos('\0', accepted->buffer, accepted->current_size);
        if (p == -1) {
            memmove(preprocessed->buffer, accepted->buffer, accepted->current_size);
            preprocessed->current_size = accepted->current_size;
            accepted->current_size = 0;
        } else {
            memmove(preprocessed->buffer, accepted->buffer, p);
            preprocessed->current_size = p;
            int deleted_string_length = p + 1;
            memmove(accepted->buffer, accepted->buffer + deleted_string_length, accepted->current_size - deleted_string_length);
            accepted->current_size -= deleted_string_length;
            *accepted_state = 0;
        }
    }
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

                buffer_t master_buffer = new_buffer(4096);
                buffer_t preprocessed_buffer = new_buffer(4096);
                buffer_t accepted_buffer = new_buffer(4096);

                short error_events = POLLERR | POLLHUP | POLLNVAL;
                int pollfds_size = 2;
                struct pollfd * pollfds = malloc_(sizeof(struct pollfd *) * pollfds_size);
                struct pollfd masterpollfd;
                masterpollfd.fd = master;
                masterpollfd.events = POLLIN | error_events;
                masterpollfd.revents = 0;
                struct pollfd acceptedpollfd;
                acceptedpollfd.fd = accepted;
                acceptedpollfd.events = POLLIN | error_events;
                acceptedpollfd.revents = 0;
                pollfds[0] = masterpollfd;
                pollfds[1] = acceptedpollfd;
                int master_eof = 0;
                int accepted_eof = 0;
                int accepted_state = 0;
                while (!master_eof || !accepted_eof || master_buffer.current_size > 0 || preprocessed_buffer.current_size > 0 || accepted_buffer.current_size > 0) {
                    check("poll", poll(pollfds, pollfds_size, -1));
                    if (pollfds[0].revents & POLLIN) {
                        read_(master, master_buffer.buffer, master_buffer.size, &master_buffer.current_size, &master_eof);
                    }
                    if (pollfds[1].revents & POLLOUT) {
                        int r = check("write", write(accepted, master_buffer.buffer, master_buffer.current_size));
                        memmove(master_buffer.buffer, master_buffer.buffer + r, master_buffer.current_size - r);
                        master_buffer.current_size -= r;
                    }
                    if (pollfds[1].revents & POLLIN) {
                        read_(accepted, accepted_buffer.buffer, accepted_buffer.size, &accepted_buffer.current_size, &accepted_eof);
                        preprocess_data(master, &accepted_buffer, &preprocessed_buffer, &accepted_state);
                    }
                    if (pollfds[0].revents & POLLOUT) {
                        int r = check("write", write(master, preprocessed_buffer.buffer, preprocessed_buffer.current_size));
                        memmove(preprocessed_buffer.buffer, preprocessed_buffer.buffer + r, preprocessed_buffer.current_size - r);
                        preprocessed_buffer.current_size -= r;
                    }
                    if (pollfds[0].revents & error_events) {
                        master_eof = 1;
                        preprocessed_buffer.current_size = 0;
                        accepted_buffer.current_size = 0;
                        accepted_eof = 1;
                    }
                    if (pollfds[1].revents & error_events) {
                        accepted_eof = 1;
                        master_buffer.current_size = 0;
                        master_eof = 1;
                    }

                    update_events(master_eof, master_buffer.current_size, master_buffer.size, &pollfds[0], &pollfds[1]);
                    update_events(accepted_eof, preprocessed_buffer.current_size, preprocessed_buffer.size, &pollfds[1], &pollfds[0]);
                }
                close_(master);
                close_(accepted);
                free(master_buffer.buffer);
                free(preprocessed_buffer.buffer);
                free(accepted_buffer.buffer);
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
