#ifndef SDAT_TABLES_H
#define SDAT_TABLES_H
#include "sdat_types.h"
extern const sdat_table original_cdt_table_frodo640, original_cdt_table_frodo976, original_cdt_table_frodo1344;
extern const sdat_table sda_table_frodo640, sda_table_frodo976, sda_table_frodo1344, sda_table_falcon_base;
const sdat_table *online_get_table(const char *family, const char *parameter_set);
int online_table_validate(const sdat_table *t);
const sdat_avx2_stats *online_avx2_stats(void); void online_avx2_stats_reset(void); void online_avx2_stats_add(uint64_t batches,uint64_t vec,uint64_t tail,uint64_t fb);
#endif
