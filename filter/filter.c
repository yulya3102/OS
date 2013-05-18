#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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

void write_(int fd, char * buf, int size)
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
                write_(1, buffer, size);
                write_(1, "\n", 1);
            }
        } while (tpid != pid);
        free(current_arg);
    }
    else {
        argv[argv_size - 2] = current_arg;
        execvp(argv[0], argv);
    }
}


int get_new_data(char * buffer, int max_size) {
    if (max_size == 0) {
        return 0;
    }
    int r = read(0, buffer, max_size);
    if (r == 0) {
        // eof
        return -1;
    }
    return r;
}

void main(int argc, char ** argv) {
    int buffer_size = 4 * 1024;
    char separator = '\n';
    int opt;
    int eof_flag = 0;
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
        //command[command_size - 2] is empty
        command[command_size - 1] = NULL;

        char * buffer = malloc(buffer_size);
        int r = get_new_data(buffer, buffer_size);
        if (r == -1) {
            eof_flag = 1;
        }
        int current_size = r;                   
        if (eof_flag) {   // input is empty
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
            if (!eof_flag) {
                // we don't have separators in buffer, so we have to read new data until we find separator
                // in buffer now (current_size - from) bytes, so we can read (buffer_size - (current_size - from)) bytes
                current_size = current_size - from;
                while (pos == -1) {
                    r = get_new_data(buffer + current_size, buffer_size - current_size);
                    if (r == -1) {
                        eof_flag = 1;
                    }
                    if (eof_flag) {
                        // r == 0 so we don't need to increase current_size
                        if (current_size > 0 && buffer[current_size - 1] != separator) {
                            // in buffer - string without separators
                            if (current_size == buffer_size) {
                                _exit(1);
                            }
                            buffer[current_size] = separator;
                            current_size = current_size + 1;
                            pos = find_separator(separator, buffer, current_size);
                        }
                        break;
                    }
                    current_size = current_size + r;
                    pos = find_separator(separator, buffer, current_size);
                    if (pos == -1 && current_size == buffer_size) {
                        _exit(1);
                    }
                }
            }
            else {
                // we don't have data in buffer and we can't read new data
                break;
            }
        }

        
        free(buffer);
        free(command);
    }
}
