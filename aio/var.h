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

    var(const var& other) = delete;

    var(var && other) = delete; // connections? changing value?
    var& operator=(const var& other) = delete;
    var& operator=(var && other) = delete;

    T operator=(T newValue) {
        if (value != newValue) {
            value = newValue;
            this->handleChange();
        }
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
