#ifndef SDAT_AVX2_H
#define SDAT_AVX2_H
#include "sdat_tables.h"
int sdat_avx2_cpu_supported(void);
int sdat_avx2_sample_batch(const sdat_table *table,sdat_randombytes_fn randombytes,void *rng_ctx,uint32_t *samples,size_t sample_count);
void sdat_avx2_lookup_u72_batch(const sdat_u72*x,size_t n,const sdat_u72*t,size_t tn,uint32_t*out);
#endif
