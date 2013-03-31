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

int main(int argc, char * argv[])
{
    //k = argv[1] ???

}
