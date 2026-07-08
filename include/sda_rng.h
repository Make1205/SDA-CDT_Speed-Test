#ifndef SDA_RNG_H
#define SDA_RNG_H
#include <stdint.h>
#include <stddef.h>
#include "sda_u128.h"
typedef int (*sda_random_bytes_fn)(void *ctx, uint8_t *out, size_t out_len);
typedef struct { uint64_t s[2]; } sda_bench_rng;
typedef struct { uint64_t accepted_samples, attempts, rejected_candidates; unsigned __int128 raw_random_bits, accepted_random_bits; } sda_rng_stats;
void sda_bench_rng_init(sda_bench_rng *r, uint64_t seed);
int sda_bench_random_bytes(void *ctx, uint8_t *out, size_t out_len);
int sda_uniform_bounded(sda_u128 q, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, sda_u128 *out);
#endif
