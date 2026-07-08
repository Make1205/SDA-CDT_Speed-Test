#ifndef SDA_GENERATION_H
#define SDA_GENERATION_H
#include <stddef.h>
#include <mpfr.h>
#include "sda_config.h"
#include "sda_table.h"
typedef struct {
  sda_u128 q,p[32],c[32]; size_t n; int q_bits;
  sda_u128 application_q,exact_svp_q,baseline_q; int application_draw_bits,threshold_bits,final_q_from_exact_svp,baseline_dominance_certified;
  mpfr_t baseline_sd_support,baseline_sd_infinite,baseline_renyi,candidate_sd_ratio,candidate_renyi_ratio,acceptance_ratio,expected_attempts,expected_raw_bits;
  sda_u128 raw_svp_q,raw_svp_p[32];
  mpfr_t max_scaled_error,max_abs_error,l1_error,sd_support,sd_infinite,tail_mass,renyi,renyi_minus_one,log2_sd,log2_renyi_minus_one,gaussian_s,raw_svp_norm,epsilon;
  unsigned long denominators_scanned; unsigned long long enumerated_q_count; double generation_time;
  int exact,heuristic,source_is_fixture;
  int denominator_search_complete,fixed_q_optimizer_certified,exact_linf_svp,global_svp_certified,raw_svp_vector_available,raw_svp_pmf_valid,pmf_is_fixed_q_normalized,production_eligible,denominator_from_exact_svp;
  int search_space_exhausted,nearest_integer_certified,norm_comparisons_certified,interval_certified,high_precision_verified,formal_certificate_valid; unsigned long long half_integer_ties;
  char solver[32];
} sda_generation_result;
void sda_generation_result_init(sda_generation_result *r, mpfr_prec_t prec);
void sda_generation_result_clear(sda_generation_result *r);
int sda_generate_distribution(const sda_config *cfg, mpfr_t *alpha, size_t n, mpfr_t tail_mass, mpfr_t gaussian_s);
int sda_fixed_q_minmax(mpfr_t *alpha, size_t n, sda_u128 q, sda_u128 *p, mpfr_t max_scaled, mpfr_t max_abs, mpfr_t l1);
int sda_search_exact_denominator(const sda_config *cfg, mpfr_t *alpha, size_t n, sda_generation_result *out);
int sda_search_application(const sda_config *cfg, mpfr_t *alpha, size_t n, sda_generation_result *out);
int sda_generate_for_config(const sda_config *cfg, const char *solver, sda_generation_result *out);
#endif
