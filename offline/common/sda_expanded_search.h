#ifndef SDA_EXPANDED_SEARCH_H
#define SDA_EXPANDED_SEARCH_H
#include <mpfr.h>
#include "sda_config.h"
typedef enum {SDA_EPS_LOG_GRID,SDA_EPS_LINEAR_GRID,SDA_EPS_RANDOM_GRID} sda_epsilon_search_mode;
typedef struct {size_t n;sda_u128 q,p[32],c[32];} sda_frozen_reference;
typedef struct {int expanded,stop_on_match;unsigned points,top_count;uint64_t seed;sda_epsilon_search_mode mode;const char*min_text,*max_text;} sda_expanded_options;
int sda_epsilon_grid_point(mpfr_t,mpfr_t,mpfr_t,unsigned,unsigned,sda_epsilon_search_mode);
int sda_expanded_stage_bounds(int,size_t,int,mpfr_t,mpfr_t,mpfr_t,mpfr_t);
void sda_expanded_basis_hash(char[17],mpfr_t*,size_t,mpfr_t,mpfr_prec_t);
int sda_frozen_equal(size_t,sda_u128,const sda_u128*,const sda_u128*,const sda_frozen_reference*);
int sda_expanded_search(const char*,const sda_expanded_options*,const sda_frozen_reference*);
#endif
