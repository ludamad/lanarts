#pragma once

template <typename T, typename Func>
bool for_each(T& value, const Func& func) {
    for (auto& subvalue : value) {
        if (!func(subvalue)) {
            return false;
        }
    }
    return true;
}