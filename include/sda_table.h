#ifndef SDA_TABLE_H
#define SDA_TABLE_H
#include <stddef.h>
#include <stdint.h>
#include "sda_u128.h"
typedef enum { SDA_WIDTH_U16=16, SDA_WIDTH_U64=64, SDA_WIDTH_U128=128 } sda_integer_width;
typedef struct { const char *scheme,*parameter_set,*solver_mode; int support_min,support_max,denominator_bits,exact,heuristic,target_q_mode; size_t table_length; sda_u128 denominator; const sda_u128 *masses; const sda_u128 *cumulative; size_t native_storage_bytes; size_t ideal_packed_bits; } sda_table;
int sda_build_cumulative(const sda_u128 *p, size_t n, sda_u128 *c, sda_u128 *q);
int sda_validate_table(const sda_table *t, char *err, size_t errn);
int sda_table_select_index(const sda_table *t, sda_u128 u);
size_t sda_table_native_bytes(const sda_table *t);
#endif
