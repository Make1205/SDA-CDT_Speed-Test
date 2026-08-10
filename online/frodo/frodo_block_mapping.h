#ifndef FRODO_BLOCK_MAPPING_H
#define FRODO_BLOCK_MAPPING_H
#include "sdat_tables.h"
#include <stddef.h>
#include <stdint.h>
int frodo_original_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n,const sdat_table*t);
int frodo_sda_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n,const sdat_table*t);
int frodo640_sda_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n);
int frodo976_sda_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n);
int frodo1344_sda_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n);
#endif
