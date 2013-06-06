#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define UNUSED(x) (void)(x)

int window_size_changed = 0;

void sigwinch_handler(int signum) {
    UNUSED(signum);
    window_size_changed = 1;
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
    char * buffer = malloc(4096);
    fcntl(socketfd, F_SETFL, O_NONBLOCK);
    fcntl(0, F_SETFL, O_NONBLOCK);
    while (1) {
        int r = read(socketfd, buffer, 4096);
        if (r > 0) {
            write(1, buffer, r);
        } else if (r == 0) {
            break;
        }

        r = read(0, buffer, 4096);
        if (r > 0) {
            write(socketfd, "t", 1);
            write(socketfd, buffer, r);
            write(socketfd, "\0", 1);
        } else if (r == 0) {
            break;
        }

        if (window_size_changed) {
            char * message = "new window size";
            write(1, message, strlen(message));
            window_size_changed = 0;
        }
    }
    return 0;
}
