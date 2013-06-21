#include "epollfd.h"
#include "buffer.h"
#include "aread.h"

#include <iostream>

int main() {
    epollfd fd;
    auto error_action = [] () { std::cout << "error" << std::endl; };
    buffer buf(0, 4096);
    while (!buf.is_eof()) {
        aread ar(fd, 0, buf, [&buf] () { std::cout << "some data" << std::endl; }, error_action);
        fd.cycle();
    }
}
