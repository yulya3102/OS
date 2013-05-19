#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define UNUSED(x) (void)(x);

int check(const char * comment, int what) {
    if (what < 0) {
        perror(comment);
        _exit(1);
    }
    return what;
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

int find_newline(char * buf, int size)
{
    if (size == 0) {
        return -1;
    }
    int pos = 0;
    while (pos < size && buf[pos] != '\n')
    {
        pos++;
    }
    if (pos == size)
    {
        return -1;
    }
    return pos;
}

// prints next string from buffer, moves buffer, returns current buffer size
int print_next_string(char * buffer, int current_size) {
    int pos = find_newline(buffer, current_size);
    int string_size = pos + 1;
    write_(1, buffer, string_size);
    write_(1, buffer, string_size);
    current_size = current_size - string_size;
    memmove(buffer, buffer + string_size, current_size);
    return current_size;
}

// read all string and delete it, then read new data
// return -1 if current string is ignored and input is empty
int ignore_string(char * buffer, int max_size) {
    // delete all data until '\n'
    int current_size = 0;
    int pos = -1;
    do {
        if (current_size == max_size) { // buffer is full, but still doesn't contain newline
            current_size = 0;
        }
        int r = check("read failed", read(0, buffer + current_size, max_size - current_size));
        current_size = current_size + r;
        pos = find_newline(buffer, current_size);
        if (r == 0) {
            if (pos == -1) { // ignore this string
                return -1;
            }
        }
    } while (pos == -1);
    int ignored_string_size = pos + 1;
    memmove(buffer, buffer + ignored_string_size, current_size - ignored_string_size);
    current_size = current_size - ignored_string_size;
    return current_size;
}

int main(int argc, char * argv[])
{
    if (argc != 2) {
        char * usage = "Usage: dupme <max string length>\n";
        write_(1, usage, strlen(usage));
        _exit(1);
    }

    int k = atoi(argv[1]);
    int size = k + 1;
    char * buffer = malloc(size);

    // read new data, find newline
    // while buffer doesn't contain newline, read new data until it's full
    // if we've found newline - print string
    // else ignore string

    int current_size = 0;
    int pos = -1;
    int eof_flag = 0;
    while (!eof_flag) { // buffer is empty and we still can read new data
        // read new data until buffer contains newline
        while (pos == -1) {
            // buffer is full, but doesn't contain newline
            // so ignore this string
            if (current_size == size) { 
                current_size = ignore_string(buffer, current_size);
                if (current_size == -1) {
                    current_size = 0;
                    break; // dirty hack
                }
            }
            // read new data
            int r = check("read failed", read(0, buffer + current_size, size - current_size));
            current_size = current_size + r;
            pos = find_newline(buffer, current_size);
            // we can't read new data so we need to break
            if (r == 0) { // eof
                eof_flag = 1;
                if (current_size == 0) {
                    break; // не айс
                }
                // if last symbol is not newline then add it to the end
                if (buffer[current_size - 1] != '\n') {
                    // buffer can't be full, see line 112
                    buffer[current_size] = '\n';
                    current_size++;
                }
                pos = find_newline(buffer, current_size);
                break; // не айс
            }
        }
        // write all data we can process
        while (pos != -1) {
            current_size = print_next_string(buffer, current_size);
            pos = find_newline(buffer, current_size);
        }
    }
    free(buffer);

    return 0;
}
