#ifndef FALCON_BASE_SAMPLER_H
#define FALCON_BASE_SAMPLER_H
#include <stddef.h>
#include <stdint.h>
#include "sdat_tables.h"

#define FALCON_BASE_SUPPORT_MAX 18u
#define FALCON_BASE_RANDOM_BYTES 9u

int falcon_original_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out);
int falcon_sda_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out, int *accepted);
int falcon_original_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats);
size_t falcon_original_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats);
int falcon_sda_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats);
size_t falcon_sda_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats);
uint64_t falcon_base_checksum(const uint32_t *out, size_t n);

typedef uint64_t (*falcon_stage_clock_fn)(void *context);
typedef struct {falcon_stage_clock_fn read;void *context;uint64_t input_cycles,mapping_cycles,outer_cycles;} falcon_stage_timing;
typedef struct {sdat_u72 *candidates;size_t capacity;} falcon_stage_workspace;
int falcon_original_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block_size,falcon_stage_workspace*workspace,sdat_stats*stats,falcon_stage_timing*timing);
int falcon_sda_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block_size,falcon_stage_workspace*workspace,sdat_stats*stats,falcon_stage_timing*timing);

#endif
