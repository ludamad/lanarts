#ifndef ITERATOR_UTILS_H
#define ITERATOR_UTILS_H

#include <tuple>

template <typename T>
struct Result {
    T value;
    bool finished;
    Result() : finished(true) {}
    Result(const T& value) : value(value), finished(false) {}
};

//template <typename T>
//inline Result<T> EndIteration() {
//    return Result<T> {T(), false};
//}

//template <typename T>
//inline Result<T> NextValue(T&& value) {
//    return Result<T> {value, true};
//}

// Just what is needed for the C++
template <typename T, typename F>
struct LambdaIterator {
    LambdaIterator(const F& lambda, Result<T>&& value) : lambda(lambda), value(value) {
    }
    LambdaIterator(const F& lambda) : lambda(lambda) {
        value = this->lambda();
    }
    bool operator!=(const LambdaIterator& other) {
        return (value.finished != other.value.finished);
    }
    void operator++() {
        value = lambda();
    }
    auto operator*() {
        return value.value;
    }
    F lambda;
    Result<T> value;
};

template <typename F>
inline auto lambda_begin(F&& lambda) {
    return LambdaIterator<decltype(lambda().value), F>(lambda);
}

template <typename F>
inline auto lambda_end(F&& lambda) {
    return LambdaIterator<decltype(lambda().value), F>(lambda, {});
}


template <typename T>
struct LambdaIterableBase {
    auto begin() const {
        return lambda_begin(((T*)this)->iterator());
    }
    auto end() const {
        return lambda_end(((T*)this)->iterator());
    }
};

#endif //ITERATOR_UTILS_H
