#ifndef SDA_RANDOM_DRIVER_H
#define SDA_RANDOM_DRIVER_H
#include <stddef.h>
#include <stdint.h>
#include <mpfr.h>
#include "sda_u128.h"
typedef struct {
 size_t n; sda_u128 q,p[32],c[32]; unsigned first_trial,occurrences,quality_flags;
 char epsilon[192]; mpfr_t pointwise,sd,rd;
 unsigned first_stage,last_stage,contiguous_run_count,last_sequence,inside_paper_interval,frozen_identical;
 mpfr_t epsilon_min,epsilon_max,frozen_l1,frozen_max,frozen_log2_q;
 unsigned mass_mismatch_count; char basis_hash[17];
} sda_ranked_candidate;
typedef struct { sda_ranked_candidate *items; size_t count,capacity; mpfr_prec_t precision; } sda_candidate_set;
int sda_candidate_set_init(sda_candidate_set*,mpfr_prec_t);
void sda_candidate_set_clear(sda_candidate_set*);
int sda_candidate_set_add(sda_candidate_set*,size_t,sda_u128,const sda_u128*,const sda_u128*,unsigned,const char*,mpfr_t,mpfr_t,mpfr_t,unsigned*);
int sda_candidate_set_observe(sda_candidate_set*,size_t,sda_u128,const sda_u128*,const sda_u128*,unsigned,unsigned,unsigned,mpfr_t,int,mpfr_t,mpfr_t,mpfr_t,unsigned,const char*,size_t*);
int sda_random_generate_config(const char*,uint64_t,unsigned,unsigned,const char*);
#endif
