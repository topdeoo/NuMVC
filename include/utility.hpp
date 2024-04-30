//
// Created by max on 04.08.22.
//
#ifndef MPGRAPHS_UTILITY_HPP
#define MPGRAPHS_UTILITY_HPP

#include "ankerl/unordered_dense.h"

namespace mpgraphs {

/// hash function that can hash std::pair
template<typename T>
struct hash : public ankerl::unordered_dense::hash<T> { };

/// hash function that can hash std::pair
template<typename T, typename U>
struct hash<std::pair<T, U>> {
    /// compute the hash value
    auto operator()(const std::pair<T, U>& pair) const noexcept {
        auto h1 = hash<T>{}(pair.first);
        auto h2 = hash<U>{}(pair.second);
        return ankerl::unordered_dense::detail::wyhash::mix(h1, h2);
    }
};

template<class K, class V>
using map = ankerl::unordered_dense::map<K, V, hash<K>>;

template<class K>
using set = ankerl::unordered_dense::set<K, hash<K>>;

struct TodoType {};

template<class...> struct PrintType;

template<class... T>
void unused(T&&...) { }
} // namespace mpgraphs
#endif //MPGRAPHS_UTILITY_HPP
