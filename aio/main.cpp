#include "epollfd.h"
#include <iostream>

int main() {
    epollfd fd;
    std::function<void()> action;
    action = [&action, &fd] () {
        std::cout << "input: ";
        std::string str;
        std::cin >> str;
        std::cout << str << std::endl;
        fd.subscribe(0, EPOLLIN, action, [] () {});
    };
    fd.subscribe(0, EPOLLIN, action, [] () {});
    while (true) {
        fd.cycle();
    }
}
