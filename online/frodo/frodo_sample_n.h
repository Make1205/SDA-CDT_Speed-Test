#ifndef FRODO_SAMPLE_N_H
#define FRODO_SAMPLE_N_H
#include <stddef.h>
#include <stdint.h>
#include "sdat_tables.h"
#include "sdat_bitreader.h"
int frodo_original_sample_n(uint16_t *s,size_t n,const sdat_table *table);
int frodo_sda_sample_n(uint16_t *out,size_t n,sdat_bitreader *reader,const sdat_table *table,sdat_stats *stats);
int frodo_original_sample_n_avx2(uint16_t *s,size_t n,const sdat_table *table);
int frodo_sda_sample_n_avx2(uint16_t *out,size_t n,sdat_bitreader *reader,const sdat_table *table,sdat_stats *stats);
size_t frodo_uniform_bounded_u16_batch(sdat_bitreader*r,uint16_t q,unsigned bits,uint16_t*a,uint8_t*signs,size_t target,sdat_stats*st);
size_t frodo_uniform_bounded_u8_batch(sdat_bitreader*r,uint8_t q,unsigned bits,uint8_t*a,uint8_t*signs,size_t target,sdat_stats*st);
uint16_t frodo_apply_sign(uint16_t mag,uint8_t sign);
uint16_t frodo_lookup_magnitude_scalar(uint32_t x,const sdat_table*t);
#endif
