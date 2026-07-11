#include "falcon_sda_sampler.h"
#include "sda_sampler.h"
#include <stdio.h>
#include <string.h>
int falcon_sda_uniform_bnd_72(sda_u128 q, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, sda_u128 *out){ if(!q || q>(((sda_u128)1)<<72)) return -1; return sda_uniform_bounded(q,fn,ctx,st,out); }
int falcon_sda_table_validate(const sda_table *t, char *err, size_t errlen){ if(!t || strcmp(t->scheme,"Falcon")){ if(err&&errlen) snprintf(err,errlen,"not a Falcon table"); return -1; } if(t->support_min!=0 || t->support_max!=18){ if(err&&errlen) snprintf(err,errlen,"Falcon base support must be 0..18"); return -2; } if(t->denominator>(((sda_u128)1)<<72)){ if(err&&errlen) snprintf(err,errlen,"denominator exceeds 72 bits"); return -3; } return sda_validate_table(t,err,errlen); }
int falcon_sda_base_sample(const sda_table *t, sda_random_bytes_fn fn, void *ctx, sda_rng_stats *st, int *out){ char e[96]; if(falcon_sda_table_validate(t,e,sizeof e)) return -1; return sda_sample_symmetric(t,fn,ctx,st,0,out); }
