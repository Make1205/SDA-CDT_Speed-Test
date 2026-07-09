#ifndef SDA_DISTRIBUTION_H
#define SDA_DISTRIBUTION_H
#include <mpfr.h>
#include <stddef.h>
#include "sda_config.h"
int sda_distribution_abs_gaussian(const sda_config *cfg, mpfr_t *probs, size_t n);
#endif
