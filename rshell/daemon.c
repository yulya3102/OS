#include "daemon.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int daemon_pid;

void proxy_handler(int signum) {
    kill(daemon_pid, signum);
}

int daemonize(bool wait_in_foreground)
{
    int i;
    for (i = 0; i < 32; ++i)
        signal(i, &proxy_handler);

    daemon_pid = fork();
    if (daemon_pid == -1)
        return -1;

    if (daemon_pid == 0)
    {
        if (setsid() == -1)
            return -1;

        return 0;
    }

    int status = 0;
    if (wait_in_foreground)
        waitpid(daemon_pid, &status, 0);

    _exit(status);
}
