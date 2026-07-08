#ifndef SDA_INTERVAL_H
#define SDA_INTERVAL_H
#include <mpfr.h>
#include <gmp.h>
typedef struct { mpfr_t lo; mpfr_t hi; } sda_mpfr_interval;
void sda_interval_init(sda_mpfr_interval *x, mpfr_prec_t p);
void sda_interval_clear(sda_mpfr_interval *x);
void sda_interval_set(sda_mpfr_interval *x, const mpfr_t v);
void sda_interval_set_ui(sda_mpfr_interval *x, unsigned long v);
void sda_interval_add(sda_mpfr_interval *r, const sda_mpfr_interval *a, const sda_mpfr_interval *b);
void sda_interval_sub(sda_mpfr_interval *r, const sda_mpfr_interval *a, const sda_mpfr_interval *b);
void sda_interval_mul(sda_mpfr_interval *r, const sda_mpfr_interval *a, const sda_mpfr_interval *b);
void sda_interval_mul_z(sda_mpfr_interval *r, const sda_mpfr_interval *a, const mpz_t z);
void sda_interval_div(sda_mpfr_interval *r, const sda_mpfr_interval *a, const sda_mpfr_interval *b);
void sda_interval_abs(sda_mpfr_interval *r, const sda_mpfr_interval *a);
void sda_interval_max(sda_mpfr_interval *r, const sda_mpfr_interval *a, const sda_mpfr_interval *b);
int sda_interval_contains(const sda_mpfr_interval *x, const mpfr_t v);
int sda_interval_disjoint(const sda_mpfr_interval *a, const sda_mpfr_interval *b);
void sda_interval_width(mpfr_t out, const sda_mpfr_interval *x);
#endif
