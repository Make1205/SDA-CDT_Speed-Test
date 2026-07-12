#ifndef FRODO_SAMPLER_H
#define FRODO_SAMPLER_H
#include "frodo_sample_n_fast.h"

typedef enum { FRODO_PARAM_640, FRODO_PARAM_976, FRODO_PARAM_1344 } frodo_param_id;
typedef enum { FRODO_BACKEND_REFERENCE, FRODO_BACKEND_AVX2 } frodo_backend;
typedef enum { FRODO_SAMPLER_ORIGINAL_CDT, FRODO_SAMPLER_SDA_CDT } frodo_sampler_kind;
typedef enum { FRODO_FRONTEND_ORIGINAL_WORD, FRODO_FRONTEND_PACKED_BIT, FRODO_FRONTEND_WORD_ORIENTED } frodo_frontend;

typedef struct {
    frodo_param_id id;
    const char *name;
    const sdat_table *original_table;
    const sdat_table *sda_table;
    uint16_t sda_q;
    unsigned sda_candidate_bits;
    unsigned sda_sign_bit;
    uint16_t sda_candidate_mask;
    const uint16_t *original_cdf;
    size_t original_cdf_len;
    const void *sda_thresholds;
    size_t sda_threshold_count;
    size_t support_max;
    size_t native_sample_count;
} frodo_sampler_params;

typedef struct {
    sdat_stats stats;
    sdat_bitreader_fast reader;
} frodo_sampler_stats;

const frodo_sampler_params *frodo_get_sampler_params(frodo_param_id id);
const char *frodo_sampler_kind_name(frodo_sampler_kind kind);
const char *frodo_backend_name(frodo_backend backend);
const char *frodo_frontend_name(frodo_frontend frontend);
const char *frodo_implementation_label(frodo_sampler_kind kind, frodo_backend backend, frodo_frontend frontend);
int frodo_backend_available(frodo_backend backend);
int frodo_sample_n_dispatch(frodo_sampler_kind kind, frodo_backend backend, frodo_frontend frontend,
                            frodo_param_id param, uint16_t *out, size_t n,
                            const uint8_t *packed_source, size_t packed_source_len,
                            const uint16_t *word_source, size_t word_count,
                            frodo_sampler_stats *stats);
#endif
