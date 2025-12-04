#include "utils.hpp"

// Parse uint128 from string
uint128_t parse_u128(const std::string& s) {
    uint128_t res = 0;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            res = res * 10 + (c - '0');
        }
    }
    return res;
}

// Print uint128
std::ostream& operator<<(std::ostream& os, uint128_t n) {
    if (n == 0) return os << "0";
    std::string s;
    while (n > 0) {
        s += (char)('0' + (n % 10));
        n /= 10;
    }
    std::reverse(s.begin(), s.end());
    return os << s;
}

// RNG Constructor
FastRNG::FastRNG() {
    s[0] = 0x123456789ABCDEF;
    s[1] = 0xFEDCBA987654321;
}

// Lemire's Method (32-bit)
uint32_t fast_uniform_bound(FastRNG& rng, uint32_t range) {
    uint64_t x = rng.next() & 0xFFFFFFFF;
    uint64_t m = x * (uint64_t)range;
    uint32_t l = (uint32_t)m;
    if (l < range) {
        uint32_t t = -range;
        if (t >= range) {
            t -= range;
            if (t >= range) t %= range;
        }
        while (l < t) {
            x = rng.next() & 0xFFFFFFFF;
            m = x * (uint64_t)range;
            l = (uint32_t)m;
        }
    }
    return m >> 32;
}

// Lemire's Method (128-bit / Rejection for Falcon)
uint128_t fast_uniform_bound_128(FastRNG& rng, uint128_t range) {
    uint128_t x;
    // Falcon q is ~72 bits. We can optimize by masking high bits to increase acceptance rate.
    // 2^73 - 1 mask
    uint128_t mask = ((uint128_t)1 << 73) - 1;
    do {
        x = rng.next128() & mask;
    } while (x >= range);
    return x;
}