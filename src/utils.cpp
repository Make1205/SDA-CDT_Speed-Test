#include "utils.hpp"

FastRNG::FastRNG() {
    s[0] = 0x123456789ABCDEF;
    s[1] = 0xFEDCBA987654321;
}

uint32_t fast_uniform_bound(FastRNG& rng, uint32_t range) {
    uint64_t x = rng.next() & 0xFFFFFFFF; // Use lower 32 bits
    uint64_t m = x * (uint64_t)range;
    uint32_t l = (uint32_t)m;
    
    // Rejection sampling (only happens rarely)
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