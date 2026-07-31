#ifndef FRODO_BLOCK_STAGED_H
#define FRODO_BLOCK_STAGED_H
#include "frodo_sampler.h"
#include <stddef.h>
#include <stdint.h>
typedef uint64_t (*frodo_stage_clock_fn)(void *context);
typedef struct {frodo_stage_clock_fn read;void *context;uint64_t input_cycles;uint64_t mapping_cycles;} frodo_stage_timing;
typedef struct {uint16_t *candidates;uint8_t *signs;size_t capacity;} frodo_stage_workspace;
int frodo_original_block_staged_sample_n(uint16_t *out,size_t n,const uint16_t *words,size_t word_count,const sdat_table *table,size_t block_size,frodo_stage_workspace *workspace,sdat_stats *stats,frodo_stage_timing *timing);
int frodo_sda_block_staged_sample_n(uint16_t *out,size_t n,const uint16_t *words,size_t word_count,const sdat_table *table,size_t block_size,frodo_stage_workspace *workspace,sdat_stats *stats,frodo_stage_timing *timing);
#endif
