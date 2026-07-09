#ifndef FALCON_SDA_SAMPLER_H
#define FALCON_SDA_SAMPLER_H
#include "sda_table.h"
#include "sda_rng.h"
int falcon_sda_uniform_bnd_72(sda_u128 q, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, sda_u128 *out);
int falcon_sda_table_validate(const sda_table *t, char *err, size_t errlen);
int falcon_sda_base_sample(const sda_table *t, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, int *out);
#endif
