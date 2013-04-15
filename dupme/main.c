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

//prints next string from buffer, moves buffer, returns size of data printed
int print_next_string(char * buf, int size)
{
    int pos = find_newline(buf, size);
    int string_size = pos + 1;
    char * str = malloc(string_size);
    str = memcpy(str, buf, string_size);
    memmove(buf, buf + string_size, size - string_size);
    _write(1, str, string_size);
    _write(1, str, string_size);
    free(str);
    return string_size;
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
        current_size = currentesize + r;
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

// TODO: rewrite it using new _read() and ignore_string()
int main(int argc, char * argv[])
{
    int k = _atoi(argv[1]);
    int size = k + 1;
    char * buffer = malloc(size);
    int expected = size;
    int res = _read(0, buffer, expected);
    //if it contains '\n' - print it
    //else ignore all string until '\n'
    int pos = find_newline(buffer, size);
    //before cycle: in buffer - new string
    //in pos - position of '\n'
    while (res == expected)
    {
        if (pos == -1) //ignore string
        {
            while (pos == -1 && res == expected)
            {
                expected = size;
                res = _read(0, buffer, expected);
                pos = find_newline(buffer, res);
            }
            //in buffer - end of ignored string + some data from stream
            //delete the rest from ignored string
            //move end of buffer to the begin
            //fill the end of buffer with new data
            //find new position of '\n'
            int string_size = pos + 1;
            memmove(buffer, buffer + string_size, size - string_size);
            if (res != expected)
            {
                expected = size;
                res = 0;
                break;
            }
            expected = string_size;
            res = _read(0, buffer + size - expected, expected);
        }
        else
        {
            //print string before '\n'
            //move the rest of string to the begin
            //fill the end of buffer with data from stdin
            //find new position of '\n'
            expected = print_next_string(buffer, size);
            res = _read(0, buffer + size - expected, expected);
        }
        //last parameter is size because outside of this cycle it will be anyway refreshed
        pos = find_newline(buffer, size);
    }
    //res != expected, in buffer some string from end of stream
    //print every string
    size = size - expected + res;
    pos = find_newline(buffer, size);

    while (pos != -1)
    {
        size -= print_next_string(buffer, size);
        pos = find_newline(buffer, size);
    }
    //in buffer some string without \n
    _write(1, buffer, size);
    _write(1, "\n", 1);
    _write(1, buffer, size);
    free(buffer);
}
