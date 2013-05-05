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

void show_unified_diff(char * first, char * second) {
    int pid = fork();
    if (pid == 0) {
        execlp("diff", "diff", "-u", first, second, NULL);
    }
    int status;
    waitpid(pid, &status, 0);
}

void main(int argc, char ** argv) {
    if (argc < 2) {
        _write(1, "Usage:\nwatchthis $interval $command", 34);
        _exit(0);
    }
    int interval = atoi(argv[0]);
    char * command = argv[1];
    char ** command_args = argv + 1;
    //old=`$@`
    //echo $old
    while (1) {
        sleep(interval);
        //new=`$@`
        //udiff=`diff -u <( echo "$old" ) <( echo "$new" )`
        //if [ "$udiff" ]; then
        //  echo $new
        //  echo $udiff
        //fi
        //old="$new"
    }
}
