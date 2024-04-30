#pragma once

#ifndef __BASIC_HPP__
#define __BASIC_HPP__

#include <ankerl/unordered_dense.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <random>
#include <unordered_map>
#include <unordered_set>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef double fp64;

double random_alpha();
u32 random_int( u32 l, u32 r );

#endif  // __BASIC_HPP__