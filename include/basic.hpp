#pragma once

#ifndef __BASIC_HPP__
#define __BASIC_HPP__

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef double fp64;

struct custom_hash {
    //! @fn splitmix64
    //! @brief hash function
    static uint64_t splitmix64( uint64_t x ) {
        x += 0x9e3779b97f4a7c15;
        x = ( x ^ ( x >> 30 ) ) * 0xbf58476d1ce4e5b9;
        x = ( x ^ ( x >> 27 ) ) * 0x94d049bb133111eb;
        return x ^ ( x >> 31 );
    }

    //! @brief get hash value
    std::size_t operator()( uint64_t x ) const {
        static const uint64_t FIXED_RANDOM =
            std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64( x + FIXED_RANDOM );
    }
};

template <typename K, typename V>
using map = std::unordered_map<K, V, custom_hash>;

template <typename K>
using set = std::unordered_set<K, custom_hash>;

#endif  // __BASIC_HPP__