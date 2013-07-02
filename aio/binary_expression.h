#pragma once

#include "expression.h"

#include <functional>

template <typename T, typename Left, typename Right>
struct binary_expression : expression<T> {
    typedef std::function<T(Left, Right)> operation_t;
    typedef typename expression<Left>::subscription_t left_subscription_t;;
    typedef typename expression<Right>::subscription_t right_subscription_t;;

    binary_expression(const binary_expression& other)
        : value(new T(*other.value))
        , operation(other.operation)
        , left(other.left)
        , right(other.right)
        , update_value([this] () {
                *value = operation(**left, **right);
                this->handleChange();
            })
        , sleft(left->subscribe(expression<Left>::any_change, update_value))
        , sright(right->subscribe(expression<Right>::any_change, update_value))
    {}

    binary_expression(binary_expression && other) = delete;

    binary_expression(expression<Left>& left, expression<Right>& right, operation_t operation)
        : value(new T(operation(*left, *right)))
        , operation(operation)
        , left(&left)
        , right(&right)
        , update_value([this, operation, &left, &right] () {
                *value = operation(*left, *right);
                this->handleChange();
            })
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
    operation_t operation;
    expression<Left> * left;
    expression<Right> * right;
    std::function<void()> update_value;
    left_subscription_t sleft;
    right_subscription_t sright;
};
