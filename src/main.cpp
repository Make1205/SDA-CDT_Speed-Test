


#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "sampler.hpp" // Includes utils.hpp automatically

// ==========================================
// 3. Benchmark Logic
// ==========================================

void run_comparison(std::string name, 
                    const std::vector<uint16_t>& std_table, int std_k,
                    const std::vector<uint16_t>& sda_table, int sda_q) {
    std::cout << "Testing " << name << "..." << std::endl;

    StandardSampler s_std(std_table, std_k);
    SDASampler s_sda(sda_table, sda_q);

    const int ITER = 10000000;
    
    // --- Speed Test: Standard ---
    // Warmup
    for(int i=0; i<1000; i++) s_std.sample();
    
    uint64_t t1 = rdtscp();
    long long checksum1 = 0;
    for(int i=0; i<ITER; i++) {
        checksum1 += s_std.sample();
    }
    uint64_t t2 = rdtscp();
    double cycle_std = (double)(t2 - t1) / ITER;

    // --- Speed Test: SDA ---
    // Warmup
    for(int i=0; i<1000; i++) s_sda.sample();

    uint64_t t3 = rdtscp();
    long long checksum2 = 0;
    for(int i=0; i<ITER; i++) {
        checksum2 += s_sda.sample();
    }
    uint64_t t4 = rdtscp();
    double cycle_sda = (double)(t4 - t3) / ITER;

    // --- Output ---
    std::cout << "  Table Size: Standard[" << std_table.size() << "] vs SDA[" << sda_table.size() << "]" << std::endl;
    std::cout << "  Speed (Cycles): " << cycle_std << " vs " << cycle_sda << std::endl;
    std::cout << "  Improvement: " << (cycle_std - cycle_sda)/cycle_std * 100.0 << "%" << std::endl;
    
    // Checksum to prevent dead code elimination
    if(checksum1 == 12345) std::cout << ""; 
    if(checksum2 == 12345) std::cout << "";
    std::cout << "--------------------------------" << std::endl;
}

int main() {
    // Data Source: Sci China Inf Sci Paper Table 6
    
    // ==========================================
    // 1. Frodo-640
    // ==========================================
    // Standard: 0..12, Precision 2^15
    std::vector<uint16_t> f640_std = {
        4643, 13363, 20579, 25843, 29227, 31145, 32103, 
        32525, 32689, 32745, 32762, 32766, 32767
    };
    // SDA: 0..11, q = 14534 (Table 6: SDAT(/14534))
    std::vector<uint16_t> f640_sda = {
        2071, 5957, 9166, 11499, 12992, 13833, 14250, 
        14432, 14502, 14526, 14533, 14534
    };
    run_comparison("Frodo-640", f640_std, 15, f640_sda, 14534);

    // ==========================================
    // 2. Frodo-976
    // ==========================================
    // Standard: 0..10, Precision 2^15
    std::vector<uint16_t> f976_std = {
        5638, 15915, 23689, 28571, 31116, 32217, 32613, 
        32731, 32760, 32766, 32767
    };

    // std::vector<uint16_t> f976_sda = {
    //     1291, 3640, 5409, 6512, 7081, 7324, 7410, 
    //     7435, 7441, 7442
    // };
    // run_comparison("Frodo-976", f976_std, 15, f976_sda, 7442);

        std::vector<uint16_t> f976_sda = {
        213, 601, 893, 1075, 1169, 1209, 1223, 
        1227, 1228
    };
    run_comparison("Frodo-976", f976_std, 15, f976_sda, 1228);

    // ==========================================
    // 3. Frodo-1344
    // ==========================================
    // Standard: 0..6, Precision 2^15
    std::vector<uint16_t> f1344_std = {
        9142, 23462, 30338, 32361, 32725, 32765, 32767
    };
    // SDA: 0..4, q = 102 (Table 6: SDAT(/102))
    // Paper: "D(+-5) and D(+-6) ... obtained via SDA are all equal to 0"
    std::vector<uint16_t> f1344_sda = {
        29, 74, 95, 101, 102
    };
    run_comparison("Frodo-1344", f1344_std, 15, f1344_sda, 102);

    return 0;
}