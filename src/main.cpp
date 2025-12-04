#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "sampler.hpp"

template <typename T>
void run_comparison(std::string name, 
                    const std::vector<T>& std_table, int std_k,
                    const std::vector<T>& sda_table, T sda_q) {
    std::cout << "Testing " << name << "..." << std::endl;

    StandardSampler<T> s_std(std_table, std_k);
    SDASampler<T> s_sda(sda_table, sda_q);

    // Calc memory bits
    // Standard: Size * k
    size_t std_bits = std_table.size() * std_k;
    // SDA: Size * log2(q). For 128-bit q, we count exact bits.
    int sda_width = 0;
    T temp_q = sda_q;
    while(temp_q > 0) { temp_q >>= 1; sda_width++; }
    size_t sda_bits = sda_table.size() * sda_width;

    const int ITER = 10000000;
    
    // --- Speed Standard ---
    for(int i=0; i<1000; i++) s_std.sample(); // Warmup
    uint64_t t1 = rdtscp();
    long long checksum1 = 0;
    for(int i=0; i<ITER; i++) checksum1 += s_std.sample();
    uint64_t t2 = rdtscp();
    double cycle_std = (double)(t2 - t1) / ITER;

    // --- Speed SDA ---
    for(int i=0; i<1000; i++) s_sda.sample(); // Warmup
    uint64_t t3 = rdtscp();
    long long checksum2 = 0;
    for(int i=0; i<ITER; i++) checksum2 += s_sda.sample();
    uint64_t t4 = rdtscp();
    double cycle_sda = (double)(t4 - t3) / ITER;

    // --- Output ---
    std::cout << "  Table Size : Standard[" << std_table.size() << "] vs SDA[" << sda_table.size() << "]" << std::endl;
    std::cout << "  Speed      : " << cycle_std << " vs " << cycle_sda << " cycles" << std::endl;
    std::cout << "  Speedup    : " << (cycle_std - cycle_sda)/cycle_std * 100.0 << "%" << std::endl;
    std::cout << "  Memory     : " << std_bits << " vs " << sda_bits << " bits" << std::endl;
    std::cout << "  Mem Reduct : " << (double)(std_bits - sda_bits)/std_bits * 100.0 << "%" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    
    if(checksum1 == 12345) std::cout << "";
    if(checksum2 == 12345) std::cout << "";
}

int main() {
    std::cout << "=== SDA-CDT Benchmark: Real Paper Data ===\n\n";

    // =========================================================
    // 1. Frodo Data (Source: Table 6 [cite: 606])
    // =========================================================
    // Frodo-640
    std::vector<uint16_t> f640_std = {4643, 13363, 20579, 25843, 29227, 31145, 32103, 32525, 32689, 32745, 32762, 32766, 32767};
    std::vector<uint16_t> f640_sda = {2071, 5957, 9166, 11499, 12992, 13833, 14250, 14432, 14502, 14526, 14533, 14534};
    run_comparison("Frodo-640", f640_std, 15, f640_sda, (uint16_t)14534);

    // Frodo-976
    std::vector<uint16_t> f976_std = {5638, 15915, 23689, 28571, 31116, 32217, 32613, 32731, 32760, 32766, 32767};
    std::vector<uint16_t> f976_sda = {1291, 3640, 5409, 6512, 7081, 7324, 7410, 7435, 7441, 7442};
    run_comparison("Frodo-976", f976_std, 15, f976_sda, (uint16_t)7442);

    // Frodo-1344
    std::vector<uint16_t> f1344_std = {9142, 23462, 30338, 32361, 32725, 32765, 32767};
    // q=102, only 5 entries (0-4) stored
    std::vector<uint16_t> f1344_sda = {29, 74, 95, 101, 102};
    run_comparison("Frodo-1344", f1344_std, 15, f1344_sda, (uint16_t)102);

    // =========================================================
    // 2. Falcon Data (Source: Table 4 [cite: 600, 601])
    // =========================================================
    // Data transcribed directly from strings in your paper
    
    // Falcon SDA Table (Right Column of Table 4)
    std::vector<std::string> falcon_sda_str = {
        "1688501996594986825177", "3140552495174822643020", "4064021121781526494454",
        "4498354148456746465294", "4649426102519045973669", "4688286310671560942787",
        "4695678731498558959720", "4696718719969437723960", "4696826920814140839268",
        "4696835245983194185188", "4696835719696686440440", "4696835739630883021131",
        "4696835740251456874874", "4696835740265738826826", "4696835740265763760555",
        "4696835740265763827799", "4696835740265763827703", "4696835740265763827899",
        "4696835740265763827900" // q_SDA
    };
    
    // Falcon Standard Table (Left Column of Table 4: Ideal CDT)
    std::vector<std::string> falcon_std_str = {
        "1697680241746640300030", "3157623698389553259646", "4086112053407564316161",
        "4522805998224618730780", "4674699139014987931793", "4713770580863280169633",
        "4721203184912300545308", "4722248826482293120038", "4722357615477842549720",
        "4722365985900287751063", "4722366462188760059397", "4722366482231313364705",
        "4722366482855042897512", "4722366482869397786949", "4722366482869642109570",
        "4722366482869645184872", "4722366482869645213498", "4722366482869645213695",
        "4722366482869645213696" // 2^72
    };

    std::vector<uint128_t> f_std_vec, f_sda_vec;
    for(const auto& s : falcon_std_str) f_std_vec.push_back(parse_u128(s));
    for(const auto& s : falcon_sda_str) f_sda_vec.push_back(parse_u128(s));
    
    uint128_t falcon_sda_q = f_sda_vec.back();

    run_comparison("Falcon", f_std_vec, 72, f_sda_vec, falcon_sda_q);

    return 0;
}