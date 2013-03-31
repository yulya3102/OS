#include <unistd.h>

void _read(int fd, char * buf, int size)
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
}

int atoi(char * pNumber)
{
    char c = *pNumber;
    int number = 0;
    while (c != 0)
    {
        number = number * 10 + c - '0';
    }
    return number;
}


int main(int argc, char * argv[])
{
    //k = argv[1] ???

}
