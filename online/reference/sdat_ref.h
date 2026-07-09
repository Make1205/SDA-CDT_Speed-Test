#ifndef SDAT_REF_H
#define SDAT_REF_H
#include "sdat_tables.h"
uint32_t sdat_ref_lookup_u8(uint8_t x,const uint8_t*t,size_t n);
uint32_t sdat_ref_lookup_u16(uint16_t x,const uint16_t*t,size_t n);
uint32_t sdat_ref_lookup_u32(uint32_t x,const uint32_t*t,size_t n);
uint32_t sdat_ref_lookup_u64(uint64_t x,const uint64_t*t,size_t n);
uint32_t sdat_ref_lookup_u72(sdat_u72 x,const sdat_u72*t,size_t n);
int sdat_ref_uniform_u8(uint8_t q,unsigned bits,sdat_randombytes_fn fn,void*ctx,uint8_t*out,sdat_stats*st);
int sdat_ref_uniform_u16(uint16_t q,unsigned bits,sdat_randombytes_fn fn,void*ctx,uint16_t*out,sdat_stats*st);
int sdat_ref_uniform_u72(sdat_u72 q,sdat_randombytes_fn fn,void*ctx,sdat_u72*out,sdat_stats*st);
int sdat_ref_sample(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *sample);
int sdat_ref_sample_batch(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *samples,size_t sample_count);
#endif
