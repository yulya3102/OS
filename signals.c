#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int sigint_exit_code = 1; // ???
int sigquit_exit_code = 0;

void print_info_about_signal(const char * signame, siginfo_t * siginfo) {
    int si_code = siginfo->si_code; // value indicating why this signal was sent
    printf("\n Signal name: %s\n", signame);
    switch (si_code) {
    case SI_USER:
        printf(" Signal was sent by kill()\n Sending process ID: %i\n Real user ID of sending process: %i\n",
                siginfo->si_pid, siginfo->si_uid);
        break;
    case SI_KERNEL:
        printf(" Signal was sent by the kernel\n");
        break;
    default:
        printf(" Signal was sent by someone\n");
        break;
    }
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

void sigcont_sigaction(int signum, siginfo_t * siginfo, void * ucontext) {
    const char signame[] = "SIGCONT";
    print_info_about_signal(signame, siginfo);
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

    struct sigaction new_sigcont_action;
    new_sigcont_action.sa_sigaction = &sigcont_sigaction;
    sigemptyset(&new_sigcont_action.sa_mask);
    new_sigcont_action.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &new_sigint_action, NULL);
    sigaction(SIGTSTP, &new_sigtstp_action, NULL);
    sigaction(SIGQUIT, &new_sigquit_action, NULL);
    sigaction(SIGCONT, &new_sigcont_action, NULL);
    while (1);
}
