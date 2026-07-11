#ifndef FRODO_SAMPLE_N_FAST_H
#define FRODO_SAMPLE_N_FAST_H
#include "frodo_sample_n.h"
#include "sdat_bitreader_fast.h"
int frodo640_sda_sample_n_scalar(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo976_sda_sample_n_scalar(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo1344_sda_sample_n_scalar(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo640_sda_sample_n_avx2(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo976_sda_sample_n_avx2(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo1344_sda_sample_n_avx2(uint16_t *out,size_t n,sdat_bitreader_fast *r,sdat_stats *st);
int frodo_sda_sample_n_fast(uint16_t *out,size_t n,sdat_bitreader_fast *r,const sdat_table *t,sdat_stats *st);
int frodo_sda_sample_n_fast_avx2(uint16_t *out,size_t n,sdat_bitreader_fast *r,const sdat_table *t,sdat_stats *st);
int frodo_sda_word_sample_n(uint16_t *out,size_t n,const uint16_t *words,size_t word_count,const sdat_table *t,sdat_stats *st);
int frodo_sda_word_sample_n_avx2(uint16_t *out,size_t n,const uint16_t *words,size_t word_count,const sdat_table *t,sdat_stats *st);
#endif
