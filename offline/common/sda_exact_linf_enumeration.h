#ifndef SDA_EXACT_LINF_ENUMERATION_H
#define SDA_EXACT_LINF_ENUMERATION_H
#include <stddef.h>
#include <mpfr.h>
#include <gmp.h>
typedef struct {
  char solver[40]; size_t dimension; mpfr_prec_t precision_used; unsigned precision_escalations;
  mpz_t *coefficients; mpfr_t *shortest_vector; mpfr_t norm_lower,norm_upper,initial_radius,final_radius;
  unsigned long long nodes_visited,leaves_visited,branches_pruned;
  int search_space_exhausted,nearest_integer_certified,norm_comparisons_certified,interval_certified,global_svp_certified;
  char failure_reason[160];
} sda_exact_linf_enum_result;
void sda_exact_linf_enum_result_init(sda_exact_linf_enum_result*r,size_t d,mpfr_prec_t p);
void sda_exact_linf_enum_result_clear(sda_exact_linf_enum_result*r);
int sda_exact_linf_enumerate(mpfr_t *B,size_t d,mpfr_prec_t p,sda_exact_linf_enum_result*r);
#endif
