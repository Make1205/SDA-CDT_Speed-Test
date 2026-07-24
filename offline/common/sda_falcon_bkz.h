#ifndef SDA_FALCON_BKZ_H
#define SDA_FALCON_BKZ_H
#include <stddef.h>
#include <stdio.h>
#include <gmp.h>
#include <stdint.h>
#include <mpfr.h>
#include "sda_u128.h"
typedef struct {sda_u128 p[19],q;int solver_status,exact_svp,heuristic_bkz,bkz_seed_controlled,selected_basis_row,recoverable_row_count,structurally_valid_row_count;char version[128],command[256];} sda_falcon_bkz_result;
int sda_falcon_bkz_available(char*,size_t);
int sda_falcon_basis_write(FILE*,mpfr_t*,mpfr_t,mpfr_prec_t,mpz_t,mpz_t[19]);
int sda_falcon_recover(const mpz_t*,const mpz_t,const mpz_t[19],sda_falcon_bkz_result*);
int sda_falcon_bkz_solve(mpfr_t*,mpfr_t,mpfr_prec_t,uint64_t,sda_falcon_bkz_result*);
#endif
