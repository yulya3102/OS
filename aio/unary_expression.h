#pragma once

#include "expression.h"

#include <functional>

template <typename T, typename Arg>
struct unary_expression : expression<T> {
    typedef std::function<T(Arg)> operation_t;
    typedef typename expression<Arg>::subscription_t subscription_t;

    unary_expression(const unary_expression& other) = delete;
    unary_expression(unary_expression && other) = delete;

    unary_expression(expression<Arg>& expr, operation_t operation)
        : value(new T(operation(*expr)))
        , expr(&expr)
        , s(expr.subscribe(expression<Arg>::any_change, [this, operation] () {
                *value = operation(**(this->expr));
                this->handleChange();
            }))
    {}

    unary_expression& operator=(unary_expression const& other) = delete;
    unary_expression& operator=(unary_expression && other) = delete;

    virtual T operator*() const {
        return *value;
    }

    ~unary_expression() {
        expr->unsubscribe(s);
        delete value;
    }

private:
    T * value;
    expression<Arg> * expr;
    subscription_t s;
};
