#ifndef SDAT_REF_H
#define SDAT_REF_H
#include "sdat_tables.h"
uint32_t online_lookup_u16(uint16_t x,const uint16_t*t,size_t threshold_count);
uint32_t online_lookup_u72(sdat_u72 x,const sdat_u72*t,size_t threshold_count);
int original_cdt_ref_sample(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *sample,sdat_stats *stats);
int original_cdt_ref_sample_batch(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *samples,size_t sample_count,sdat_stats *stats);
int sda_cdt_ref_sample(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *sample,sdat_stats *stats);
int sda_cdt_ref_sample_batch(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *samples,size_t sample_count,sdat_stats *stats);
int sdat_ref_sample(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *sample);
int sdat_ref_sample_batch(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *samples,size_t sample_count);
#endif
