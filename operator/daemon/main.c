#include <unistd.h>
#include <pty.h>
#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
    pid_t pid = fork();
    if (pid) {
        sleep(100);
    } else 
    {
        setsid(); 
        int master, slave;
        char buf[4096];
        int fd = openpty(&master, &slave, buf, NULL, NULL);
        perror(buf);
        int ff = open(buf, O_RDWR);
        close(ff);
        perror("bar");
        sleep(100);
    }
    return 0;
}
