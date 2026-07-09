#ifndef SDAT_TABLES_H
#define SDAT_TABLES_H
#include "sdat_types.h"
extern const sdat_table sdat_table_frodo640, sdat_table_frodo976, sdat_table_frodo1344, sdat_table_falcon_base;
const sdat_table *sdat_get_table(const char *parameter_set);
int sdat_table_validate(const sdat_table *t);
uint64_t sdat_avx2_path_counter(void); void sdat_avx2_path_counter_reset(void);
#endif
