#include "epollfd.h"
#include <iostream>

int main() {
    epollfd fd;
    while (true) {
        fd.subscribe(0, EPOLLIN, [] () { 
                std::cout << "input: "; 
                std::string str;
                std::cin >> str;
                std::cout << str << std::endl;
            }, [] () {} );
        fd.cycle();
    }
}
