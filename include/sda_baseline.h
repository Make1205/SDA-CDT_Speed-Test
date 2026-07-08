#ifndef SDA_BASELINE_H
#define SDA_BASELINE_H
#include <stddef.h>
#include "sda_u128.h"
const sda_u128 *sda_frodo_original_pmf(const char *parameter_set, size_t *n, sda_u128 *q);
int sda_frodo_original_available(const char *parameter_set);
#endif
