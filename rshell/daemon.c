#include "daemon.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

int daemon_pid;

void proxy_handler(int signum) {
    kill(daemon_pid, signum);
}

typedef void (*sighandler_t) (int);

int daemonize(bool wait_in_foreground)
{
    int i;
    sighandler_t old_handlers[32];
    for (i = 0; i < 32; ++i)
    {
        old_handlers[i] = signal(i, &proxy_handler);
        if (old_handlers[i] == SIG_ERR)
            perror("signal");
    }

    daemon_pid = fork();
    if (daemon_pid == -1)
        return -1;

    if (daemon_pid == 0)
    {
        if (setsid() == -1)
            return -1;

        for (i = 0; i < 32; ++i)
            if (signal(i, old_handlers[i]) == SIG_ERR)
                perror("signal");

        return 0;
    }

    int status = 0;
    if (wait_in_foreground)
        waitpid(daemon_pid, &status, 0);

    _exit(status);
}
