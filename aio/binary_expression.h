#pragma once

#include "expression.h"

#include <functional>

template <typename T>
struct binary_expression : expression<T> {
    typedef std::function<T(T, T)> operation_t;
    typedef typename expression<T>::subscription_t subscription_t;

    binary_expression(const binary_expression& other) = delete;
    binary_expression(binary_expression && other) = delete;

    binary_expression(expression<T>& left, expression<T>& right, operation_t operation)
        : value(new T(operation(*left, *right)))
        , update_value([this, operation] () { *value = operation(**(this->left), **(this->right)); })
        , left(&left)
        , right(&right)
        , sleft(left.subscribe(expression<T>::any_change, update_value))
        , sright(right.subscribe(expression<T>::any_change, update_value))
    {}

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
    expression<T> * left;
    expression<T> * right;
    subscription_t sleft, sright;
};
