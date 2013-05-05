// #include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

void _write(int fd, char * buf, int size)
{
    int done = 0;
    while (done < size)
    {
        int res = write(fd, buf + done, size - done);
        if (res == -1)
        {
            perror("write failed");
            _exit(1);
        }
        else
        {
            done = done + res;
        }
    }
}

const char * old = "/tmp/watchthis_old";
const char * new = "/tmp/watchthis_new";

void show_unified_diff(const char * first, const char * second) {
    int pid = fork();
    if (pid == 0) {
        if (execlp("diff", "diff", "-u", first, second, NULL) == -1) {
            perror("exec failed");
        }
    }
    int status;
    waitpid(pid, &status, 0);
}

void run(char * command, char ** argv, const char * output_file) {
    int pid = fork();
    if (pid == 0) {
        // int fd = creat(output_file, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
        int fd = creat(output_file, 0x1a4);
        dup2(fd, 1);
        if (execvp(command, argv) == -1) {
            perror("exec failed");
        }
    }
    int status;
    waitpid(pid, &status, 0);
}

void main(int argc, char ** argv) {
    if (argc < 2) {
        _write(1, "Usage:\nwatchthis $interval $command", 34);
        _exit(0);
    }
    int interval = atoi(argv[1]);
    char * command = argv[2];
    char ** command_args = argv + 2;
    run(command, command_args, old);
    //echo $old
    while (1) {
        sleep(interval);
        run(command, command_args, new);
        //echo $new
        show_unified_diff(old, new);
        if (rename(new, old) == -1) {
            perror("rename failed");
        }
    }
}
