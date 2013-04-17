#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <deque>

// find symbol in buffer and return its position, or return -1 if buffer doesn't contain it
int find_symbol(char symbol, char * buffer, int size) {
    if (size > 0) {
        int i;
        for (i = 0; i < size; i++) {
            if (buffer[i] == symbol) {
                return i;
            }
        }
    }
    return -1;
}

void run(std::deque<char *> &argv) {
    char ** command = malloc((argv.size() + 1) * sizeof(char *));
    for (int i = 0; i < argv.size(); i++) {
        command[i] = argv[i];
    }
    command[argv.size()] = NULL;
    int pid = fork();
    if (pid == 0) {
        if (execvp(argv[0], command) == -1) {
            perror("exec failed");
            _exit(1);
        }
    }
    else {
        pid_t tpid;
        int status;
        do {
            tpid = wait(&status);
        } while (tpid != pid);
        free(command);
        _exit(0);
    } 
}

void process_deque(std::deque<std::deque<char *> >& argv) {
    if (argv.size() < 1) {
        write(1, "error", 5);
        _exit(1);
    }
    if (argv.size() == 1) {
        run(argv[0]);
    }
    else {
        int pipefd[2];
        pipe(pipefd);

        if (fork() == 0) {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);

            run(argv[0]);
        }
        else {
            dup2(pipefd[0], 0);
            close(pipefd[0]);
            close(pipefd[1]);
            argv.pop_front();
            process_deque(argv);
        }
    }
}

std::deque<char *> split(char symbol, char * string, int size) {
    std::deque<char *> result;
    int pos = find_symbol(symbol, string, size);
    while (pos != -1) {
        int token_size = pos;
        char * token = malloc(token_size + 1);
        token[token_size] = 0;
        memcpy(token, string, token_size);
        result.push_back(token);
        string = string + token_size + 1;
        size = size - token_size - 1;
        pos = find_symbol(symbol, string, size);
    }
    char * last_token = malloc(size + 1);
    last_token[size] = 0;
    memcpy(last_token, string, size);
    result.push_back(last_token);
    return result;
}

std::deque<std::deque<char *> > split(char symbol, std::deque<char *> string) {
    std::deque<std::deque<char *> > result;
    std::deque<char *> * token = new std::deque<char *>();
    for (int i = 0; i < string.size(); i++) {
        if (string[i][0] != symbol) {
            token->push_back(string[i]);
        }
        else {
            result.push_back(*token);
            token = new std::deque<char *>();
        }
    }
    result.push_back(*token);
    return result;
}

void process_line(char * string, int size) {
    std::deque<char *> str = split(' ', string, size);
    char * dir = str[0];
    str.pop_front();
    str.pop_front();
    std::deque<std::deque<char *> > commands = split('|', str);
    if (fork() == 0) {
        if (chdir(dir) == -1) {
            perror("chdir failed");
            _exit(1);
        }
        process_deque(commands);
    }
}

int eof = 0;
int read_line(int fd, char * buffer, int max_size) {
    if (max_size == 0) {
        return 0;
    }
    char newline = '\n';
    int pos = -1;
    int from = 0;
    while (pos == -1) {
        int r = read(fd, buffer + from, max_size - from);
        if (r == 0) {
            eof = 1;
            if (buffer[from - 1] != newline) {
                buffer[from] = newline;
                from = from + 1;
            }
            return from;
        }
        pos = find_symbol(newline, buffer + from, r);
        from = from + r;
        if (from == max_size && pos == -1) {
            write(1, "buffer is full", 14);
            _exit(1);
        }
    }
    return from;
}

int main(int argc, char ** argv) {
    int fd = 0;
    int max_size = 4 * 1024;
    char newline = '\n';
    char * buffer = malloc(max_size * sizeof(char));
    int current_size = read_line(fd, buffer, max_size);
    int pos = find_symbol(newline, buffer, current_size);
    while (pos != -1) {
        while (pos != -1) { // process all data
            process_line(buffer, pos);
            int line_size = pos + 1;
            memmove(buffer, buffer + line_size, current_size - line_size);
            current_size = current_size - line_size;
            pos = find_symbol(newline, buffer, current_size);
        }
        // read new data
        if (eof) {
            break;
        }
        int r = read_line(fd, buffer + current_size, max_size - current_size);
        current_size = current_size + r;
        pos = find_symbol(newline, buffer, current_size);
    }
    free(buffer);
}
