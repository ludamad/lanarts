#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>

// From https://stackoverflow.com/questions/14945223/map-function-with-c11-constructs
//
// Takes an iterable, applies a function to every element,
// and returns a vector of the results
//
template <typename T, typename Func>
auto map_container(const T& iterable, Func&& func) ->
std::vector<decltype(func(std::declval<typename T::value_type>()))>
{
    // Some convenience type definitions
    typedef decltype(func(std::declval<typename T::value_type>())) value_type;
    typedef std::vector<value_type> result_type;

    // Prepares an output vector of the appropriate size
    result_type res(iterable.size());

    // Let std::transform apply `func` to all elements
    // (use perfect forwarding for the function object)
    std::transform(
        begin(iterable), end(iterable), res.begin(),
        std::forward<Func>(func)
    );

    return res;
}
