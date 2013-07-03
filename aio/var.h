#pragma once

#include "expression.h"

template <typename T>
struct var : expression<T> {
    typedef typename expression<T>::predicate_t predicate_t;
    typedef typename expression<T>::continuation_t continuation_t;
    typedef typename expression<T>::connection_t connection_t;

    var()
        : value(T())
    {}

    var(T value)
        : value(value)
    {}

    /*
    var(T && value)
        : value(std::move(value))
    {}
    */

    var(const var& other)
        : value(other.value)
    {}

    var(var && other) = delete; // connections? changing value?
    var& operator=(var && other) = delete;
    var& operator=(const var& other) = delete; // manager?

    T operator=(T newValue) {
        value = newValue;
        this->handleChange();
        return **this;
    }

    /*
    T operator=(T && newValue) {
        if (value != newValue) {
            value = newValue;
            handleChange();
        }
        return **this;
    }
    */

    virtual T operator*() const {
        return value;
    }

    ~var() = default;

private:
    T value;
};
