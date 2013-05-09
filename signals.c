#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int sigint_exit_code = 1; // ???
int sigquit_exit_code = 0;

void print_info_about_signal(const char * signame, siginfo_t * siginfo) {
    printf("\n \
            Signal name: %s\n \
            Sending process ID: %i\n \
            Real user ID of sending process: %i\n",
            signame, siginfo->si_pid, siginfo->si_uid);
}

void sigint_sigaction(int signum, siginfo_t * siginfo, void * ucontext) {
    const char signame[] = "SIGINT";
    print_info_about_signal(signame, siginfo);
    _exit(sigint_exit_code);
}

void sigtstp_sigaction(int signum, siginfo_t * siginfo, void * ucontext) {
    const char signame[] = "SIGTSTP";
    print_info_about_signal(signame, siginfo);
    kill(0, SIGSTOP);
}

void sigquit_sigaction(int signum, siginfo_t * siginfo, void * ucontext) {
    const char signame[] = "SIGQUIT";
    print_info_about_signal(signame, siginfo);
    _exit(sigquit_exit_code);
}

int main(int argc, char ** agrv) {
    struct sigaction new_sigint_action;
    new_sigint_action.sa_sigaction = &sigint_sigaction;
    sigemptyset(&new_sigint_action.sa_mask); // blocked signals
    new_sigint_action.sa_flags = SA_SIGINFO;

    struct sigaction new_sigtstp_action;
    new_sigtstp_action.sa_sigaction = &sigtstp_sigaction;
    sigemptyset(&new_sigtstp_action.sa_mask);
    new_sigtstp_action.sa_flags = SA_SIGINFO;

    struct sigaction new_sigquit_action;
    new_sigquit_action.sa_sigaction = &sigquit_sigaction;
    sigemptyset(&new_sigquit_action.sa_mask);
    new_sigquit_action.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &new_sigint_action, NULL);
    sigaction(SIGTSTP, &new_sigtstp_action, NULL);
    sigaction(SIGQUIT, &new_sigquit_action, NULL);
    while (1);
}
