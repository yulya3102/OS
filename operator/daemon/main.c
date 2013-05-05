#include <unistd.h>

int main() {
    setpgid(0,0);
    sleep(100);
    return 0;
}
