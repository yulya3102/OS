#include "copy.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdexcept>

#define LISTEN_BACKLOG 10
#define UNUSED(x) (void)(x)

/*
#include "var.h"
#include "unary_expression.h"
#include "binary_expression.h"

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
    std::cout << "subscribing to val > 10" << std::endl;
    s = a.subscribe(p, cont);
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
    unary_expression<int> b(a, [] (int value) { return value; });
    std::cout << "b == " << *b << std::endl;
    a = 14;
    std::cout << "b == " << *b << std::endl;
    binary_expression<int> c(a, b, [] (int l, int r) { return l + r; });
    auto print_c = [&c] () {std::cout << "c = a + b = " << *c << std::endl; };
    print_c();
    a = 1;
    print_c();
    a = 10;
    print_c();
}
*/

int check(std::string message, int result) {
    if (result == -1) {
        throw std::runtime_error(message + ": " + std::string(strerror(errno)));
    }
    return result;
}

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
    int socketfd = check("socket", socket(result->ai_family, result->ai_socktype, result->ai_protocol));
    int option = 1;
    check("setsockopt", setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *) &option, sizeof(option)));
    check("bind", bind(socketfd, result->ai_addr, result->ai_addrlen));
    free(result);
    result = nullptr;
    check("listen", listen(socketfd, LISTEN_BACKLOG));
    printf("waiting for connection\n");
    epoll e;
    auto cont = [&e, error_action] (int fd) {
        autofd socketfd(fd);
        printf("connected\n");
        copy(0, *socketfd, 4 * 1024 * 1024, 6 * 1024 * 1024);
    };
    e.accept(socketfd, nullptr, nullptr, cont, error_action, [] () {});
    e.cycle();
}
