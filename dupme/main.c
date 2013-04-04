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
    return string_size;
}

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
            if (find_newline(buffer, size) == k)
            {
                _write(1, "hello", 5);
            }
                _write(1, "hello", 5);
            sleep(1);
            _write(1, buffer, size);
            break;
            while (pos == 0)
            {
                expected = size;
                res = _read(0, buffer, expected);
                pos = find_newline(buffer, res);
            }
            //??
        }
        else
        {
            //print string before '\n'
            //move the rest of string to the begin
            //fill the end of buffer with data from stdin
            //find new position of '\n'
            expected = print_next_string(buffer, size);
            res = _read(0, buffer + size - expected, expected);
            pos = find_newline(buffer, size);
        }
    }
    //res != expected, in buffer some string from end of stream
    //print every string
    size = size - expected + res;
    while (pos != -1)
    {
        size -= print_next_string(buffer, size);
        pos = find_newline(buffer, size);
    }
    //in buffer some string without \n
    _write(1, buffer, size);
    _write(1, "\n", 1);
    _write(1, buffer, size);


        

    

}
