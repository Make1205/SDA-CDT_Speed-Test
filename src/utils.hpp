#pragma once
#include <cstdint>
#include <x86intrin.h>

// ==========================================
// 1. High-Performance Infrastructure
// ==========================================

// XorShift128+ RNG (Much faster than std::mt19937)
struct FastRNG {
    uint64_t s[2];
    
    FastRNG();
    
    // Returns a 64-bit random number
    inline uint64_t next() {
        uint64_t x = s[0];
        uint64_t const y = s[1];
        s[0] = y;
        x ^= x << 23;
        s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
        return s[1] + y;
    }
};

// Lemire's "Almost Divisionless" method for UniformBnd(q)
// Generates a uniform integer in [0, range-1] without using expensive '%' or '/' instructions.
uint32_t fast_uniform_bound(FastRNG& rng, uint32_t range);

// CPU Cycle Counter
inline uint64_t rdtscp() {
    unsigned int dummy;
    return __rdtscp(&dummy);
}