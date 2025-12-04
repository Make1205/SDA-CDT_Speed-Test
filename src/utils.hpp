#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <x86intrin.h>

// 128-bit Integer Support
typedef unsigned __int128 uint128_t;

// Helper: Parse string to uint128_t
uint128_t parse_u128(const std::string& s);

// Helper: Print uint128_t
std::ostream& operator<<(std::ostream& os, uint128_t n);

// XorShift128+ RNG (Fastest for benchmarking)
struct FastRNG {
    uint64_t s[2];
    
    FastRNG();
    
    // 64-bit Random
    inline uint64_t next() {
        uint64_t x = s[0];
        uint64_t const y = s[1];
        s[0] = y;
        x ^= x << 23;
        s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
        return s[1] + y;
    }

    // 128-bit Random (Constructed from 2 calls)
    inline uint128_t next128() {
        uint64_t a = next();
        uint64_t b = next();
        return ((uint128_t)a << 64) | b;
    }
};

// Lemire's Method for 32-bit bound
uint32_t fast_uniform_bound(FastRNG& rng, uint32_t range);

// Lemire's Method for 128-bit bound (Specialized for Falcon)
uint128_t fast_uniform_bound_128(FastRNG& rng, uint128_t range);

// CPU Cycle Counter
inline uint64_t rdtscp() {
    unsigned int dummy;
    return __rdtscp(&dummy);
}