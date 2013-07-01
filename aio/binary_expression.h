#pragma once

#include "expression.h"

#include <functional>

template <typename T, typename Left, typename Right>
struct binary_expression : expression<T> {
    typedef std::function<T(Left, Right)> operation_t;
    typedef typename expression<Left>::subscription_t left_subscription_t;;
    typedef typename expression<Right>::subscription_t right_subscription_t;;

    binary_expression(const binary_expression& other) = delete;
    binary_expression(binary_expression && other) = delete;

    binary_expression(expression<Left>& left, expression<Right>& right, operation_t operation)
        : value(new T(operation(*left, *right)))
        , update_value([this, operation, &left, &right] () {
                *value = operation(*left, *right);
                this->handleChange();
            })
        , left(&left)
        , right(&right)
        , sleft(left.subscribe(expression<Left>::any_change, update_value))
        , sright(right.subscribe(expression<Right>::any_change, update_value)) {
    }

    binary_expression& operator=(binary_expression const& other) = delete;
    binary_expression& operator=(binary_expression && other) = delete;

    virtual T operator*() const {
        return *value;
    }

    ~binary_expression() {
        left->unsubscribe(sleft);
        right->unsubscribe(sright);
        delete value;
    }

private:
    T * value;
    std::function<void()> update_value;
    expression<Left> * left;
    expression<Right> * right;
    left_subscription_t sleft;
    right_subscription_t sright;
};
