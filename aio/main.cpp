#include "epollfd.h"
#include "buffer.h"
#include "aread.h"
#include "awrite.h"

#include <iostream>

int main() {
    epollfd fd;
    auto error_action = [] () { std::cout << "error" << std::endl; };
    buffer buf(4096);
    aread ar(fd, 0, buf, [] () {}, error_action);
    fd.cycle();
    awrite aw(fd, 1, buf, [] () {}, error_action);
    fd.cycle();
}
