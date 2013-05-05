#include <unistd.h>
#include <signal.h>
#include <pty.h>
#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

void handler(int signum)
{
    int fd = open("/tmp/daemon.sig", O_WRONLY | O_CREAT, 0644);
    const char buf[] = "sughup\n";
    write(fd, buf, strlen(buf));
    close(fd);
}

int main() {
    pid_t pid = fork();
    if (pid) {
        sleep(100);
    } else 
    {
        int master, slave;
        char buf[4096];
        signal(SIGHUP, &handler);
        int fd = openpty(&master, &slave, buf, NULL, NULL);
        if (fork())
        {
            close(slave);
            sleep(10);
            close(master);
        } else 
        {
            close(slave);
            close(master);
            setsid(); 
            perror(buf);
            int ff = open(buf, O_RDWR);
            close(ff);
            perror("bar");
            sleep(100);
        }
    }
    return 0;
}
