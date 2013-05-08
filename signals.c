#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int sigint_exit_code = 1; // ???

void full_write(int fd, const char * buffer, int length) {
    if (length > 0) {
        int done = 0;
        while (done < length) {
            int result = write(fd, buffer + done, length - done);
            if (result == -1) {
                perror("write failed");
                _exit(1);
            }
            done = done + result;
        }
    }
}

void sigint_handler(int signum) {
    int fd = open("/dev/tty", O_WRONLY);
    if (fd == -1) {
        perror("opening tty failed");
        _exit(1);
    }
    const char buf[] = "SIGINT\n";
    full_write(fd, buf, strlen(buf));
    if (close(fd) == -1) {
        perror("closing tty failed");
        _exit(1);
    }
    _exit(sigint_exit_code);
}

int main(int argc, char ** agrv) {
    signal(SIGINT, &sigint_handler);
    while (1);
}
