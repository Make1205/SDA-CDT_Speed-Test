#ifndef FALCON_BASE_SAMPLER_H
#define FALCON_BASE_SAMPLER_H
#include <stddef.h>
#include <stdint.h>
#include "sdat_tables.h"
#include "falcon_reverse_tail.h"

#define FALCON_BASE_SUPPORT_MAX 18u
#define FALCON_BASE_RANDOM_BYTES 9u

int falcon_original_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out);
int falcon_sda_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out, int *accepted);
uint32_t falcon_sda_gaussian0_cumulative_lookup_for_test(sdat_u72 x);
uint32_t falcon_sda_gaussian0_reflected_lookup_for_test(sdat_u72 x);
int falcon_original_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats);
size_t falcon_original_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats);
int falcon_sda_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats);
size_t falcon_sda_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats);
size_t falcon_original_gaussian0_sample_n_from_le9(const uint8_t *raw, size_t raw_len,
                                                    uint32_t *out, size_t n,
                                                    size_t *attempts);
size_t falcon_sda_gaussian0_sample_n_from_le9(const uint8_t *raw, size_t raw_len,
                                               uint32_t *out, size_t n,
                                               size_t *attempts);
uint64_t falcon_base_checksum(const uint32_t *out, size_t n);

typedef uint64_t (*falcon_stage_clock_fn)(void *context);
typedef struct {falcon_stage_clock_fn read;void *context;uint64_t input_cycles,mapping_cycles,outer_cycles;} falcon_stage_timing;
typedef struct {falcon_u72_limbs *candidates;size_t capacity;} falcon_stage_workspace;
typedef enum {
    FALCON_SDA_INPUT_CURRENT_REFLECTED = 0,
    FALCON_SDA_INPUT_DIRECT_TAIL = 1,
    FALCON_SDA_INPUT_DIRECT_OPT_COMPARE = 2,
    FALCON_SDA_INPUT_DIRECT_OPT_INPUT = 3
} falcon_sda_input_audit_variant;
typedef enum {
    FALCON_ORIGINAL_INPUT_REFERENCE_DECODE = 0,
    FALCON_ORIGINAL_INPUT_INLINE_DECODE = 1,
    FALCON_ORIGINAL_INPUT_DIRECT_STORE = 2,
    FALCON_ORIGINAL_INPUT_FULLY_OPTIMIZED = 3
} falcon_original_input_audit_variant;
size_t falcon_original_input_prepare_for_audit(
    falcon_original_input_audit_variant variant, const uint8_t *raw, size_t raw_len,
    falcon_u72_limbs *out, size_t n);
size_t falcon_original_sample_raw_for_audit(
    falcon_original_input_audit_variant variant, const uint8_t *raw, size_t raw_len,
    uint32_t *out, size_t n);
size_t falcon_sda_input_prepare_for_audit(
    falcon_sda_input_audit_variant variant, const uint8_t *raw, size_t raw_len,
    falcon_u72_limbs *out, size_t n, size_t *attempts);
size_t falcon_sda_sample_raw_for_audit(
    falcon_sda_input_audit_variant variant, const uint8_t *raw, size_t raw_len,
    uint32_t *out, size_t n, size_t *attempts);
int falcon_original_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block_size,falcon_stage_workspace*workspace,sdat_stats*stats,falcon_stage_timing*timing);
int falcon_sda_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block_size,falcon_stage_workspace*workspace,sdat_stats*stats,falcon_stage_timing*timing);

#endif
