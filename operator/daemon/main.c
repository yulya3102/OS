#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid) {
        sleep(100);
    } else 
    {
        setsid(); 
        sleep(100);
    }
    return 0;
}
