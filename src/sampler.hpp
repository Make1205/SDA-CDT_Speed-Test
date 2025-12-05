#pragma once
#include <vector>
#include <type_traits>
#include "utils.hpp"

// ==========================================
// Standard Sampler
// ==========================================
template <typename T>
struct StandardSampler {
    const std::vector<T>& table;
    T mask;
    FastRNG rng;

    StandardSampler(const std::vector<T>& t, int k) : table(t) {
        if constexpr (std::is_same<T, uint128_t>::value) {
            mask = ((uint128_t)1 << k) - 1;
        } else {
            mask = ((T)1 << k) - 1;
        }
    }

    __attribute__((always_inline)) int sample() {
        T u;
        uint64_t rand_bits; // Used for sign
        
        if constexpr (std::is_same<T, uint128_t>::value) {
            uint128_t r = rng.next128();
            u = r & mask;
            rand_bits = (uint64_t)(r >> 64);
        } else {
            rand_bits = rng.next();
            u = (T)rand_bits & mask;
        }
        
        int x = -1;
        for (const auto& val : table) {
            x += (u < val);
        }
        
        return (rand_bits >> 63) ? -x : x;
    }
};

// ==========================================
// SDA Sampler
// ==========================================
template <typename T>
struct SDASampler {
    const std::vector<T>& table;
    T q;
    T mask; 
    FastRNG rng;

    SDASampler(const std::vector<T>& t, T _q) : table(t), q(_q) {
        
        if constexpr (std::is_same<T, uint128_t>::value) {
            int bits = 0;
            T temp = q;
            while(temp > 0) { temp >>= 1; bits++; }
            mask = ((T)1 << bits) - 1;
        } else {
            mask = 0;
        }
    }

    __attribute__((always_inline)) int sample() {
        T u;
        uint64_t sign_bit = 0;


        if constexpr (std::is_same<T, uint128_t>::value) {
            
            
            while (true) {
                uint128_t r = rng.next128(); 
                u = r & mask;                
                
                // 检查是否在范围内
                if (u < q) {
                    
                    sign_bit = (uint64_t)(r >> 64); 
                    break; 
                }
                
            }
        } 

        else {
            uint64_t rand_val = rng.next();
            uint32_t r_in = (uint32_t)rand_val; 
            uint64_t m = r_in * (uint64_t)q;
            uint32_t l = (uint32_t)m;
            
            if (__builtin_expect(l < q, 0)) {
                uint32_t t = -q;
                if (t >= q) { t -= q; if (t >= q) t %= q; }
                while (l < t) {
                    r_in = rng.next() & 0xFFFFFFFF;
                    m = r_in * (uint64_t)q;
                    l = (uint32_t)m;
                }
            }
            u = m >> 32;
            sign_bit = rand_val;
        }

        int x = -1;
        for (const auto& val : table) {
            x += (u < val);
        }
        
        return (sign_bit >> 63) ? -x : x;
    }
};