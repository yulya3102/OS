#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int _read(int fd, char * buf, int size)
{
    int done = 0;
    while (done < size)
    {
        int res = read(fd, buf + done, size - done);
        if (res == -1)
        {
            perror("read failed");
            _exit(1);
        }
        else if (res == 0)
        {
            break;
        }
        else
        {
            done = done + res;
        }
    }
    return done;
}

int find_separator(char separator, char * buf, int size)
{
    int pos = 0;
    while (pos < size && buf[pos] != separator)
    {
        pos++;
    }
    if (pos == size)
    {
        return -1;
    }
    return pos;
}

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

void run(char ** argv, int argv_size, char * buffer, int size) {
    char * current_arg = malloc(size + 1);
    current_arg = memcpy(current_arg, buffer, size);
    current_arg[size] = 0;
    int pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return;
    } if (pid) {
        pid_t tpid;
        int child_status;
        do {
            tpid = wait(&child_status);
            if (WIFEXITED(child_status) && !WEXITSTATUS(child_status)) {
                _write(1, buffer, size);
                _write(1, "\n", 1);
            }
        } while (tpid != pid);
        free(current_arg);
    }
    else {
        argv[argv_size - 2] = current_arg;
        execvp(argv[0], argv);
    }
}

void main(int argc, char ** argv) {
    int buffer_size = 4 * 1024;
    char separator = '\n';
    int opt;
    while ((opt = getopt(argc, argv, "nzb:")) != -1) {
        switch (opt) {
            case 'n':
                separator = '\n';
                break;
            case 'z':
                separator = '\0';
                break;
            case 'b':
                buffer_size = atoi(optarg);
                break;
        }
    }
    char ** command;
    //command = argv + optind, arg, NULL
    int command_size = argc - optind + 2;
    if (command_size > 1) {
        command = malloc((command_size) * sizeof(char *));
        int i;
        for (i = optind; i < argc; i++) {
            command[i - optind] = argv[i];
        }
        //command[command_size - 1] is empty
        command[command_size] = NULL;

        char * buffer = malloc(buffer_size);
        int eof = 0;
        int r = read(0, buffer, buffer_size);   
        int current_size = r;                   
        if (r == 0) {   //input is empty
            return;
        }

        int pos = find_separator(separator, buffer, r);

        while (pos != -1) { // we still can print data from buffer

            // remove from buffer every string we can process
            int from = 0;
            while (pos != -1) {
                run(command, command_size, buffer + from, pos);
                from = from + pos + 1;
                pos = find_separator(separator, buffer + from, current_size - from);
            }
            
            // read new data
            memmove(buffer, buffer + from, current_size - from);
            if (!eof) {
                r = _read(0, buffer + (current_size - from), from); // _read because we really need new data
                if (r < from) {
                    eof = 1;
                    current_size = current_size - from + r;
                    buffer[current_size] = separator;
                    current_size = current_size + 1;
                }
                else {
                    current_size = current_size - from + r;
                }
            }
            else {
                current_size = current_size - from;
            }

            pos = find_separator(separator, buffer, current_size);
        }

        
        free(buffer);
        free(command);
    }
}
