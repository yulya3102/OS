/*
#include "epollfd.h"
#include "buffer.h"
#include "aread.h"
#include "awrite.h"
#include "aaccept.h"
#include "epoll.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define LISTEN_BACKLOG 10
#define UNUSED(x) (void)(x)
*/

#include "var.h"

#include <functional>
#include <iostream>
#include <list>

int main() {
    var<int> a(10);
    // var<int> b = 10; — tries to call var(const var& other)
    var<int>::predicate_t a_predicate([] (int val) { return val > 5; });
    var<int>::continuation_t a_cont([] () { std::cout << "val > 5" << std::endl; });
    var<int>::subscription_t s = a.subscribe(a_predicate, a_cont);
    //std::cout << "subscribed to a > 5" << std::endl;
    a = 6;
    a.unsubscribe(s);
    std::cout << "unsubscribed from a > 5" << std::endl;
    a = 7;
    auto predicate = [] (int val) { return val > 10; };
    var<int>::predicate_t p(predicate);
    std::function<void()> cont = [&a] () {
        std::cout << "val = " << *a << std::endl;
    };
    var<int>::continuation_t c = cont;
    std::cout << "subscribing to val > 10" << std::endl;
    s = a.subscribe(p, c);
    std::cout << "subscribed to val > 10" << std::endl;
    a = 10;
    std::cout << "a == 10" << std::endl;
    a = 11;
    std::cout << "a == 11" << std::endl;
    a = 12;
    std::cout << "a == 12" << std::endl;
    a.unsubscribe(s);
    a = 13;
    std::cout << "a == 13" << std::endl;
    //var<int> b(15);
    //binary_expression<int> expr(a, b, binary_expression<int>::any_change, [&a, &b] () { std::cout << "a + b = " << *a + *b << std::endl; });
    //a = 0;
    //b = 2;
    //a = 5;
    //b = 10;
}
/*
int main() {
    auto error_action = [] () { std::cout << "error" << std::endl; };
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo * result;
    if (getaddrinfo(0, "8822", &hints, &result) != 0) {
        error_action();
        _exit(1);
    }
    int socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    int option = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option));
    bind(socketfd, result->ai_addr, result->ai_addrlen);
    free(result);
    result = nullptr;
    listen(socketfd, LISTEN_BACKLOG);
    printf("waiting for connection\n");
    epoll e;
    auto cont = [&e, error_action] (int fd) {
        buffer buf(4096);
        auto read_cont = [&e, fd, &buf, error_action] () {
            e.write(fd, buf, [] () {}, error_action);
            e.cycle();
        };
        e.read(0, buf, read_cont, error_action);
        e.cycle();
    };
    e.accept(socketfd, nullptr, nullptr, cont, error_action);
    e.cycle();
}
*/
