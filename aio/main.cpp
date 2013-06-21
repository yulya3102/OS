#include "epollfd.h"
#include <iostream>

int main() {
    epollfd fd;
    std::function<void()> action;
    auto error_action = [] () { std::cout << "error" << std::endl; };
    bool eof = false;
    char * buffer = (char *) malloc(4096);
    action = [&] () {
        int r = read(0, buffer, 4096);
        if (r == -1) {
            perror("read failed");
            _exit(1);
        } else if (r == 0) {
            eof = true;
            write(1, "eof\n", 4);
        } else {
            write(1, "input: ", 7);
            write(1, buffer, r);
            fd.subscribe(0, EPOLLIN, action, error_action);
        }
    };
    fd.subscribe(0, EPOLLIN, action, error_action);
    while (!eof) {
        fd.cycle();
    }
}
