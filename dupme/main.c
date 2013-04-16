#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

int EOF = 0;

// read data and set EOF
int _read(char * buffer, int max_size)
{
    if (max_size == 0) {
        return 0;
    }
    int done = read(0, buffer, max_size);
    if (done == -1) {
        perror("read failed");
        _exit(1);
    }
    if (done == 0) {
        EOF = 1;
    }
    return done;
}

int _atoi(char * pNumber)
{
    char c = *pNumber;
    int number = 0;
    while (c != 0)
    {
        number = number * 10 + c - '0';
        pNumber++;
        c = *pNumber;
    }
    return number;
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
    _write(1, buffer, string_size);
    _write(1, buffer, string_size);
    current_size = current_size - string_size;
    memmove(buffer, buffer + string_size, current_size);
    return current_size;
}

// read all string and delete it, then read new data
int ignore_string(char * buffer, int max_size) {
    // delete all data until '\n'
    int current_size = 0;
    int pos = -1;
    while (pos == -1) {
        if (current_size == max_size) { // buffer is full, but still doesn't contain newline
            current_size = 0;
        }
        int r = _read(buffer + current_size, max_size - current_size);
        current_size = current_size + r;
        pos = find_newline(buffer, current_size);
        if (EOF) {
            if (pos == -1) { // ignore this string
                return 0;
            }
            else {
                break;
            }
        }
    }
    int ignored_string_size = pos + 1;
    memmove(buffer, buffer + ignored_string_size, current_size - ignored_string_size);
    current_size = current_size - ignored_string_size;
    return current_size;
}

int main(int argc, char * argv[])
{
    int k = _atoi(argv[1]);
    int size = k + 1;
    char * buffer = malloc(size);

    // read new data, find newline
    // while buffer doesn't contain newline, read new data until it's full
    // if we've found newline - print string
    // else ignore string

    int current_size = 0;
    int pos = -1;
    while (!EOF) { // buffer is empty and we still can read new data
        // read new data until buffer contains newline
        while (pos == -1) {
            // buffer is full, but doesn't contain newline
            // so ignore this string
            if (current_size == size) { 
                current_size = ignore_string(buffer, current_size);
            }
            // read new data
            int r = _read(buffer + current_size, size - current_size);
            current_size = current_size + r;
            pos = find_newline(buffer, current_size);
            // we can't read new data so we need to break
            if (EOF) {
                if (current_size == 0) {
                    break;
                }
                // if last symbol is not newline then add it to the end
                if (buffer[current_size] != '\n') {
                    // buffer can be full
                    // in this case print next string and add newline
                    // or ignore last string if it doesn't contain newline
                    if (current_size == size) {
                        if (pos != -1) {
                            current_size = print_next_string(buffer, current_size);
                            if (current_size > 0) { // buffer is not empty
                                buffer[current_size] = '\n';
                                current_size = current_size + 1;
                            }
                        }
                        else {
                            current_size = ignore_string(buffer, current_size);
                        }
                    }
                    else {
                        buffer[current_size] = '\n';
                        current_size = current_size + 1;
                    }
                }
                pos = find_newline(buffer, current_size);
                break;
            }
        }
        // write all data we can process
        while (pos != -1) {
            current_size = print_next_string(buffer, current_size);
            pos = find_newline(buffer, current_size);
        }
    }
    free(buffer);
}
