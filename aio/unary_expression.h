#pragma once

#include "expression.h"

#include <functional>

template <typename T>
struct unary_expression : expression<T> {
    typedef std::function<T(T)> operation_t;
    typedef typename expression<T>::subscription_t subscription_t;

    unary_expression(const unary_expression& other) = delete;
    unary_expression(unary_expression && other) = delete;

    unary_expression(expression<T>& expr, operation_t operation)
        : value(new T(operation(*expr)))
        , expr(&expr)
        , s(expr.subscribe(expression<T>::any_change, [this, operation] () { *value = operation(**(this->expr)); }))
    {}

    unary_expression& operator=(unary_expression const& other) = delete;
    unary_expression& operator=(unary_expression && other) = delete;

    virtual T operator*() const {
        return *value;
    }

    ~unary_expression() {
        expr->unsubscribe(s);
    }

private:
    T * value;
    expression<T> * expr;
    subscription_t s;
};