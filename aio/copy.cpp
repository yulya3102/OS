#include "copy.h"
#include "epoll.h"
#include "var.h"
#include "binary_expression.h"

#include <sys/timerfd.h>
#include <stdexcept>
#include <functional>

static void start_timer(int timerfd) {
    struct itimerspec new_value;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    new_value.it_value.tv_sec = now.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec;
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;
    timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, nullptr);
}

static void stop_timer(int timerfd) {
    struct itimerspec new_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 0;
    timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &new_value, nullptr);
}

copy::copy(int fromfd, int tofd, int speed, int maxspeed)
    : timerfd(timerfd_create(CLOCK_REALTIME, 0))
    , error_cont([] () { throw new std::runtime_error("copying failed"); }) {

    var<uint64_t>::predicate_t is_positive = [] (uint64_t value) { return value > 0; };
    expression<bool>::predicate_t is_true ([] (bool value) { return value; });

    int current_size = 0;
    var<uint64_t> exp(0);
    var<int> current_speed(speed);
    var<bool> eof(false);
    bool subscribed_to_timerfd = false;
    var<bool> subscribed_to_read(false), subscribed_to_write(false);
    buffer timerbuf(sizeof(uint64_t)), databuf(maxspeed);

    binary_expression<bool, int, bool> can_be_read(current_speed, eof,
            [] (int current_speed, bool eof) { return (current_speed > 0) && !eof; }
        );
    binary_expression<bool, bool, bool> can_be_subscribed_to_read(
            can_be_read,
            subscribed_to_read,
            [] (bool can_be_read, bool subscribed_to_read) {
                return can_be_read && !subscribed_to_read;
            }
        );
    binary_expression<bool, int, bool> can_be_subscribed_to_write(
            databuf.size(),
            subscribed_to_write,
            [] (int current_size, bool subscribed_to_write) {
                return current_size > 0 && !subscribed_to_write;
            }
        );

    auto subscribe_to_read = [&current_size, this, fromfd, &databuf, &current_speed,&subscribed_to_read, &can_be_subscribed_to_read, &eof] () {
            e.read(fromfd, databuf, *current_speed, [ &current_size, &databuf, &current_speed, &subscribed_to_read ] () {
                    int diff = *databuf.size() - current_size;
                    current_speed = *current_speed - diff;
                    current_size = *databuf.size();
                    subscribed_to_read = false;
                },
                error_cont,
                [&eof] () { eof = true; });
            subscribed_to_read = true;
        };
    auto subscribe_to_write = [&current_size, this, tofd, &databuf, &subscribed_to_write] () {
            e.write(tofd, databuf, [&current_size, &databuf, &subscribed_to_write] () {
                    current_size = *databuf.size();
                    subscribed_to_write = false;
                },
                error_cont,
                [] () {});
            subscribed_to_write = true;
        };
    auto subscribe_to_timerfd = [this, &timerbuf, &exp, &subscribed_to_timerfd] () {
        e.read(timerfd, timerbuf, [this, &timerbuf, &exp, &subscribed_to_timerfd] () {
                if (*(timerbuf.size()) != sizeof(uint64_t)) {
                    error_cont();
                }
                uint64_t d;
                timerbuf.write(&d, sizeof(uint64_t));
                exp = d;
                subscribed_to_timerfd = false;
            },
            error_cont,
            [] () {});
        subscribed_to_timerfd = true;
    };

    can_be_subscribed_to_read.subscribe(is_true, subscribe_to_read);
    can_be_subscribed_to_write.subscribe(is_true, subscribe_to_write);

    auto timer_action = [&current_speed, &exp, speed, maxspeed] () {
        int cur = *current_speed;
        current_speed = 0;
        current_speed = std::min(cur + speed, maxspeed);
        exp = 0;
    };
    exp.subscribe(is_positive, timer_action);

    start_timer(timerfd);
    while (!(*eof) || *databuf.size() > 0) {
        if (!subscribed_to_timerfd) {
            subscribe_to_timerfd();
        }
        e.cycle();
    }
    stop_timer(timerfd);
}
