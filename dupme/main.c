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
            char * str = malloc(pos + 1);
            str = memcpy(str, buffer, pos + 1);
            _write(1, str, pos + 1);
            //_write(1, str, pos + 1);
            buffer = memmove(buffer, buffer + pos + 1, size - pos - 1);
            expected = pos + 1;
            res = _read(0, buffer + size - pos - 1, expected);
            pos = find_newline(buffer, size);
        }
    }
    //res != size, in buffer some string from end of stream
    _write(1, buffer, res);
    //_write(1, buffer, res);

        

    

}
