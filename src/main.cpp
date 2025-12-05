#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "sampler.hpp"

// =========================================================
// 1. Frodo & Falcon 对比函数 (Standard CDT vs SDA-CDT)
// =========================================================
template <typename T, size_t StdSize, size_t SdaSize>
void run_unified_benchmark(std::string name, 
                           const std::vector<T>& std_table, int std_k,
                           const std::vector<T>& sda_table, T sda_q) {
    std::cout << "Testing " << name << " (Std CDT vs SDA)..." << std::endl;

    StandardSampler<T, StdSize> s_std(std_table, std_k);
    SDASampler<T, SdaSize>      s_sda(sda_table, sda_q);

    // Memory calculation
    size_t std_bits = std_table.size() * std_k;
    int sda_width = 0;
    T temp_q = sda_q;
    while(temp_q > 0) { temp_q >>= 1; sda_width++; }
    size_t sda_bits = sda_table.size() * sda_width;

    const int ITER = 10000000;
    
    // Speed Standard
    for(int i=0; i<1000; i++) s_std.sample(); 
    uint64_t t1 = rdtscp();
    long long checksum1 = 0;
    for(int i=0; i<ITER; i++) checksum1 += s_std.sample();
    uint64_t t2 = rdtscp();
    double cycle_std = (double)(t2 - t1) / ITER;

    // Speed SDA
    for(int i=0; i<1000; i++) s_sda.sample(); 
    uint64_t t3 = rdtscp();
    long long checksum2 = 0;
    for(int i=0; i<ITER; i++) checksum2 += s_sda.sample();
    uint64_t t4 = rdtscp();
    double cycle_sda = (double)(t4 - t3) / ITER;

    std::cout << "  [Baseline] Standard CDT Speed : " << cycle_std << " cycles\n";
    std::cout << "  [Proposed] SDA-CDT Speed      : " << cycle_sda << " cycles\n";
    std::cout << "  Speedup                       : " << (cycle_std - cycle_sda)/cycle_std * 100.0 << "%\n";
    std::cout << "  Memory Reduction              : " << std_bits << " bits -> " << sda_bits << " bits\n";
    std::cout << "--------------------------------\n";
    
    if(checksum1 == 12345) std::cout << "";
    if(checksum2 == 12345) std::cout << "";
}

// =========================================================
// 2. Kyber CBD 专用对比函数 (Algorithmic vs SDA-CDT)
//    这里不再生成 "Standard CDT"，而是直接对比官方算法
// =========================================================
template <int Eta, size_t SdaSize>
void run_cbd_comparison(std::string name, 
                        const std::vector<uint16_t>& sda_table, 
                        uint16_t sda_q) {
    std::cout << "Testing " << name << " (Algorithmic vs SDA)..." << std::endl;
    
    // 1. Baseline: 纯算法实现 (位运算)
    CBDSampler<Eta> cbd_alg;
    
    // 2. Proposed: SDA 查表实现
    SDASampler<uint16_t, SdaSize> cbd_sda(sda_table, sda_q);

    // 计算 SDA 内存
    int sda_width = 0;
    uint16_t temp_q = sda_q;
    while(temp_q > 0) { temp_q >>= 1; sda_width++; }
    size_t sda_bits = sda_table.size() * sda_width;

    const int ITER = 10000000;

    // --- Benchmark Algorithmic ---
    for(int i=0; i<1000; i++) cbd_alg.sample();
    uint64_t t1 = rdtscp();
    long long checksum1 = 0;
    for(int i=0; i<ITER; i++) checksum1 += cbd_alg.sample();
    uint64_t t2 = rdtscp();
    double cycle_alg = (double)(t2 - t1) / ITER;

    // --- Benchmark SDA ---
    for(int i=0; i<1000; i++) cbd_sda.sample();
    uint64_t t3 = rdtscp();
    long long checksum2 = 0;
    for(int i=0; i<ITER; i++) checksum2 += cbd_sda.sample();
    uint64_t t4 = rdtscp();
    double cycle_sda = (double)(t4 - t3) / ITER;

    std::cout << "  [Baseline] Algorithmic Speed  : " << cycle_alg << " cycles\n";
    std::cout << "  [Proposed] SDA-CDT Speed      : " << cycle_sda << " cycles\n";
    
    // 注意：SDA 通常比位运算慢，所以这里展示差异而非“加速”
    double diff = (cycle_sda - cycle_alg) / cycle_alg * 100.0;
    std::cout << "  Performance Diff              : " << (diff > 0 ? "+" : "") << diff << "% (Cycles)\n";
    
    // 内存对比：算法没有表，所以内存是 0 vs SDA
    std::cout << "  Memory Usage                  : 0 bits (Code) vs " << sda_bits << " bits (Table)\n";
    std::cout << "--------------------------------\n";

    if(checksum1 == 12345) std::cout << "";
    if(checksum2 == 12345) std::cout << "";
}

int main() {
    std::cout << "=== SDA-CDT Benchmarking Suite ===\n\n";

    // =========================================================
    // 1. Kyber CBD Comparisons (Algorithmic vs SDA)
    // =========================================================
    
    // CBD2: Weights [4, 5, 1], Sum=10
    // CDF = [4, 9, 10]
    // Size = 3 (0, 1, 2)
    std::vector<uint16_t> cbd2_sda = {4, 9, 10};
    // run_cbd_comparison<2, 3>("Kyber-CBD2", cbd2_sda, 10);

    // CBD3: Weights [10, 14, 5, 1], Sum=30
    // CDF = [10, 24, 29, 30]
    // Size = 4 (0, 1, 2, 3)
    std::vector<uint16_t> cbd3_sda = {10, 24, 29, 30};
    // run_cbd_comparison<3, 4>("Kyber-CBD3", cbd3_sda, 30);


    std::cout << "\n=== Frodo & Falcon Comparisons (CDT vs SDA) ===\n";

    // =========================================================
    // 2. Frodo-640 (Table 6)
    // =========================================================
    std::vector<uint16_t> f640_std = {
        4643, 13363, 20579, 25843, 29227, 31145, 32103, 
        32525, 32689, 32745, 32762, 32766, 32767
    };
    std::vector<uint16_t> f640_sda = {
        2071, 5957, 9166, 11499, 12992, 13833, 14250, 
        14432, 14502, 14526, 14533, 14534
    };
    run_unified_benchmark<uint16_t, 13, 12>("Frodo-640", f640_std, 15, f640_sda, (uint16_t)14534);

    // =========================================================
    // 3. Frodo-976 (Table 6)
    // =========================================================
    std::vector<uint16_t> f976_std = {
        5638, 15915, 23689, 28571, 31116, 32217, 32613, 
        32731, 32760, 32766, 32767
    };
    // std::vector<uint16_t> f976_sda = {
    //     1291, 3640, 5409, 6512, 7081, 7324, 7410, 
    //     7435, 7441, 7442
    // };
    // run_unified_benchmark<uint16_t, 11, 10>("Frodo-976", f976_std, 15, f976_sda, (uint16_t)7442);
    std::vector<uint16_t> f976_sda = {
        213, 601, 893, 1075, 1169, 1209, 1223, 
        1227, 1228
    };
    run_unified_benchmark<uint16_t, 11, 9>("Frodo-976", f976_std, 15, f976_sda, (uint16_t)1228);

    // =========================================================
    // 4. Frodo-1344 (Table 6)
    // =========================================================
    std::vector<uint16_t> f1344_std = {
        9142, 23462, 30338, 32361, 32725, 32765, 32767
    };
    std::vector<uint16_t> f1344_sda = {
        29, 74, 95, 101, 102
    };
    run_unified_benchmark<uint16_t, 7, 5>("Frodo-1344", f1344_std, 15, f1344_sda, (uint16_t)102);

    // =========================================================
    // 5. Falcon (Table 4)
    // =========================================================
    std::vector<std::string> falcon_sda_str = {
        "1688501996594986825177", "3140552495174822643020", "4064021121781526494454",
        "4498354148456746465294", "4649426102519045973669", "4688286310671560942787",
        "4695678731498558959720", "4696718719969437723960", "4696826920814140839268",
        "4696835245983194185188", "4696835719696686440440", "4696835739630883021131",
        "4696835740251456874874", "4696835740265738826826", "4696835740265763760555",
        "4696835740265763827799", "4696835740265763827703", "4696835740265763827899",
        "4696835740265763827900" 
    };
    std::vector<std::string> falcon_std_str = {
        "1697680241746640300030", "3157623698389553259646", "4086112053407564316161",
        "4522805998224618730780", "4674699139014987931793", "4713770580863280169633",
        "4721203184912300545308", "4722248826482293120038", "4722357615477842549720",
        "4722365985900287751063", "4722366462188760059397", "4722366482231313364705",
        "4722366482855042897512", "4722366482869397786949", "4722366482869642109570",
        "4722366482869645184872", "4722366482869645213498", "4722366482869645213695",
        "4722366482869645213696"
    };

    std::vector<uint128_t> f_std_vec, f_sda_vec;
    for(const auto& s : falcon_std_str) f_std_vec.push_back(parse_u128(s));
    for(const auto& s : falcon_sda_str) f_sda_vec.push_back(parse_u128(s));
    uint128_t falcon_sda_q = f_sda_vec.back();

    run_unified_benchmark<uint128_t, 19, 19>("Falcon (N=19)", f_std_vec, 72, f_sda_vec, falcon_sda_q);

    return 0;
}