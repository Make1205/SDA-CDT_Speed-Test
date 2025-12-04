#pragma once
#include <vector>
#include "utils.hpp"

// ==========================================
// 2. Sampler Core Logic
// ==========================================

// Standard CDT Sampler (Power-of-two denominator)
struct StandardSampler {
    const std::vector<uint16_t>& table;
    uint32_t mask;
    FastRNG rng;

    StandardSampler(const std::vector<uint16_t>& t, int k) : table(t), mask((1<<k)-1) {}

    __attribute__((always_inline)) int sample() {
        
        uint64_t rand_val = rng.next();
        
        
        uint32_t u = (uint32_t)rand_val & mask;
        
        int x = -1; 
        for (auto val : table) {
            x += (u < val);
        }

        
        return (rand_val >> 63) ? -x : x;
    }
};

// SDA CDT Sampler (Arbitrary denominator q)
struct SDASampler {
    const std::vector<uint16_t>& table;
    uint32_t q;
    FastRNG rng;

    SDASampler(const std::vector<uint16_t>& t, uint32_t _q) : table(t), q(_q) {}

    __attribute__((always_inline)) int sample() {
        
        uint64_t rand_val = rng.next();

        
        uint32_t r_in = (uint32_t)rand_val; 
        uint64_t m = r_in * (uint64_t)q;
        uint32_t l = (uint32_t)m;

        
        if (__builtin_expect(l < q, 0)) { 
            
            uint32_t t = -q;
            if (t >= q) {
                t -= q;
                if (t >= q) t %= q;
            }
            while (l < t) {
                
                r_in = rng.next() & 0xFFFFFFFF; 
                m = r_in * (uint64_t)q;
                l = (uint32_t)m;
            }
        }
        uint32_t u = m >> 32; 


        int x = -1;
        for (auto val : table) {
            x += (u < val);
        }


        return (rand_val >> 63) ? -x : x;
    }
};