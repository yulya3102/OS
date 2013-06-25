#pragma once

#include <functional>
#include <stack>
#include <list>

namespace details {
    template <typename T>
    struct manager_t;

    template <typename T>
    struct subscription_t;
}

template <typename T>
struct expression {
    typedef std::function<bool(T)> predicate_t;
    typedef std::function<void()> continuation_t;
    typedef std::function<void(T)> connection_t;
    typedef details::subscription_t<T> subscription_t;
    typedef details::manager_t<connection_t> manager_t;
    typedef typename std::list<connection_t>::iterator connection_iterator;

    subscription_t subscribe(predicate_t predicate, continuation_t cont) {
        connections.push_front([predicate, cont] (T value) {
                if (predicate(value)) {
                    cont();
                }
            });
        return subscription_t(connections.begin(), predicate, cont);
    };

    void unsubscribe(subscription_t s) {
        connection_iterator it = s.it;
        (*it) = nullptr;
    }

    virtual T operator*() const = 0;

    const static predicate_t any_change;

protected:
    void handleChange() {
        for (connection_iterator it = connections.begin(); it != connections.end(); ) {
            connection_iterator oldIt = it;
            ++it;
            manager.addEvent(*oldIt);
        }
        manager.run(this);
        if (!manager.isRunning()) {
            for (connection_iterator it = connections.begin(); it != connections.end(); ) {
                connection_iterator oldIt = it;
                ++it;
                if (*oldIt == nullptr) {
                    connections.erase(oldIt);
                }
            }
        }
    }

private:
    manager_t manager;
    std::list<connection_t> connections;
};

namespace details {
    template <typename T>
    struct manager_t {
        manager_t()
            : isRunning_(false)
        {}

        manager_t(const manager_t&) = delete;
        manager_t(manager_t &&) = delete;
        manager_t& operator=(const manager_t&) = delete;
        manager_t& operator=(manager_t &&) = delete;

        void addEvent(T const& connection) {
            if (connection != nullptr) {
                pending.push(connection);
            }
        }

        bool isRunning() const {
            return isRunning_;
        }

        template<typename This>
        void run(This value) {
            if (!isRunning_) {
                isRunning_ = true;
                while (!pending.empty()) {
                    T connection = pending.top();
                    connection(**value);
                    pending.pop();
                }
                isRunning_ = false;
            }
        }

        ~manager_t() = default;

    private:
        std::stack<T> pending;
        bool isRunning_;
    };

    template <typename T>
    struct subscription_t {
        friend struct expression<T>;
        typedef typename expression<T>::predicate_t predicate_t;

        typedef typename expression<T>::connection_iterator iterator_t;
        typedef std::function<void()> continuation_t;

        subscription_t(iterator_t it, predicate_t predicate, continuation_t cont)
            : it(it)
            , predicate(predicate)
            , cont(cont)
        {}

    private:
        iterator_t it;
        predicate_t predicate;
        std::function<void()> cont;
    };
}

template <typename T>
const typename expression<T>::predicate_t expression<T>::any_change = predicate_t([] (T) { return true; });
