#ifndef SDA_SAMPLER_H
#define SDA_SAMPLER_H
#include "sda_table.h"
#include "sda_rng.h"
int sda_sample_magnitude(const sda_table *t, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, int *out);
int sda_sample_symmetric(const sda_table *t, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, int consume_zero_sign, int *out);
#endif
