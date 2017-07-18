#ifndef ITERATOR_UTILS_H
#define ITERATOR_UTILS_H

// Just what is needed for the C++
template <typename F>
struct LambdaIterator {
    LambdaIterator(const F& lambda, bool is_end) : lambda(lambda), is_end(is_end) {
    }

    bool operator!=(const LambdaIterator& other) {
        return (is_end != other.is_end);
    }
    void operator++(int) {
        // a no-op
    }
    auto operator*() {
        return lambda(is_end);
    }
    F lambda;
    bool is_end;
};

template <typename F>
inline auto lambda_iterator(const F& lambda, bool is_end) {
    return LambdaIterator<F>(lambda, is_end);
}

template <typename T>
struct LambdaIterableBase {
    auto begin() {
        return lambda_iterator(((T*)this)->iterator(), false);
    }
    auto end() {
        return lambda_iterator(((T*)this)->iterator(), true);
    }
};

#endif //ITERATOR_UTILS_H
