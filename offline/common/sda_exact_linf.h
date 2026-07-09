#ifndef SDA_EXACT_LINF_H
#define SDA_EXACT_LINF_H
#include <stddef.h>
typedef struct { unsigned long long nodes_visited,leaves_visited,branches_pruned; long initial_radius,final_radius,best_norm; long best_coefficients[8],best_vector[8]; double elapsed_time; } sda_linf_stats;
int sda_exact_linf_solve(const long *B, size_t d, sda_linf_stats *st);
long sda_linf_bruteforce(const long *B, size_t d, int bound);
#endif
