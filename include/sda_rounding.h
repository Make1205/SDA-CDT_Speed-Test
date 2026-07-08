#ifndef SDA_ROUNDING_H
#define SDA_ROUNDING_H
#include <mpfr.h>
#include <stddef.h>
#include "sda_u128.h"
int sda_balanced_rounding(mpfr_t *alpha, size_t n, sda_u128 q, sda_u128 *p);
#endif
