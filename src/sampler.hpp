#pragma once
#include <vector>
#include <type_traits>
#include "utils.hpp"

// ==========================================
// Standard Sampler (Bitmask)
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
// SDA Sampler (Optimized Masked Rejection)
// ==========================================
template <typename T>
struct SDASampler {
    const std::vector<T>& table;
    T q;
    T mask; // 预计算的掩码
    FastRNG rng;

    SDASampler(const std::vector<T>& t, T _q) : table(t), q(_q) {
        // 预计算 Mask：找到比 q 大的最小 2 的幂减 1
        if constexpr (std::is_same<T, uint128_t>::value) {
            // Falcon q ~ 72 bits. 
            // 简单起见，我们直接计算最高位
            int bits = 0;
            T temp = q;
            while(temp > 0) { temp >>= 1; bits++; }
            // 如果 q 正好是 2^k (很少见)，mask 就是 q-1
            // 否则 mask 是 2^bits - 1
            mask = ((T)1 << bits) - 1;
        } else {
            // Frodo logic (32/64 bit) usually doesn't need mask storage
            // but for consistency we can init it.
            mask = 0;
        }
    }

    __attribute__((always_inline)) int sample() {
        T u;
        uint64_t sign_bit = 0;

        // ---------------------------------------------------------
        // Falcon 特化优化：Masked Rejection Sampling
        // ---------------------------------------------------------
        if constexpr (std::is_same<T, uint128_t>::value) {
            // 优化原理：Falcon 的 q 非常接近 2^72。
            // 直接用 mask 生成随机数，只有极低概率 (0.5%) 会失败重试。
            // 这避免了 Lemire 算法昂贵的 128 位乘法。
            
            while (true) {
                uint128_t r = rng.next128(); // 获取 128 位随机数
                u = r & mask;                // 极速位运算截断 (例如 73 bits)
                
                // 检查是否在范围内
                if (u < q) {
                    // 成功！复用刚才那个随机数的高位做符号
                    sign_bit = (uint64_t)(r >> 64); 
                    break; 
                }
                // 失败：重试 (概率 < 1%)
            }
        } 
        // ---------------------------------------------------------
        // Frodo 逻辑：保持 Lemire 算法 (对 32/64 位非常快)
        // ---------------------------------------------------------
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