#ifndef SDA_METRICS_H
#define SDA_METRICS_H
#include <mpfr.h>
#include <stddef.h>
#include "sda_u128.h"
typedef struct { mpfr_t max_scaled_error,max_absolute_error,l1_error,sd_support,sd_infinite,tail_mass,renyi,renyi_minus_one; int renyi_infinite; } sda_metrics;
void sda_metrics_init(sda_metrics *m, mpfr_prec_t prec); void sda_metrics_clear(sda_metrics *m);
int sda_compute_metrics(mpfr_t *alpha, size_t n, const sda_u128 *p, sda_u128 q, long renyi_order, sda_metrics *m);
#endif
