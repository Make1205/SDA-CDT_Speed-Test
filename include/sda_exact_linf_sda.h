#ifndef SDA_EXACT_LINF_SDA_H
#define SDA_EXACT_LINF_SDA_H
#include <stddef.h>
#include <mpfr.h>
#include "sda_u128.h"

typedef struct {
  size_t n;
  mpfr_prec_t precision;
  mpfr_t epsilon;
  mpfr_t C;
  mpfr_t norm_lower;
  mpfr_t norm_upper;
  sda_u128 q;
  sda_u128 p[32];
  unsigned long long q_enumerated;
  unsigned long long candidates_evaluated;
  unsigned long long half_integer_ties;
  unsigned precision_escalations;
  unsigned long q_search_lower;
  unsigned long q_search_upper;
  int q_zero_considered;
  int search_space_exhausted;
  int nearest_integer_certified;
  int norm_comparisons_certified;
  int interval_certified;
  int exact_linf_svp;
  int global_svp_certified;
  int high_precision_verified;
  int formal_certificate_valid;
  char failure_reason[160];
} sda_exact_linf_sda_result;

void sda_exact_linf_sda_init(sda_exact_linf_sda_result *r, size_t n, mpfr_prec_t prec);
void sda_exact_linf_sda_clear(sda_exact_linf_sda_result *r);
int sda_exact_linf_sda_solve(mpfr_t *alpha, size_t n, mpfr_t epsilon, sda_u128 initial_q, sda_exact_linf_sda_result *r);
int sda_exact_linf_sda_verify(mpfr_t *alpha, size_t n, const sda_exact_linf_sda_result *r);
#endif
