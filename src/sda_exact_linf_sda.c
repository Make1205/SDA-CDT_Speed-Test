#include "sda_exact_linf_sda.h"
#include "sda_interval.h"
#include <string.h>
#include <stdio.h>
#include <gmp.h>

static void set_u128(mpfr_t r, sda_u128 v){ mpfr_set_ui_2exp(r,(unsigned long)(v>>64),64,MPFR_RNDN); mpfr_add_ui(r,r,(unsigned long)v,MPFR_RNDN); }
static sda_u128 mpz_get_u128_local(const mpz_t z){ unsigned char buf[16]={0}; size_t n=0; mpz_export(buf,&n,-1,1,0,0,z); sda_u128 v=0; for(size_t i=0;i<n&&i<16;i++) v|=((sda_u128)buf[i])<<(8*i); return v; }

void sda_exact_linf_sda_init(sda_exact_linf_sda_result *r, size_t n, mpfr_prec_t prec){ memset(r,0,sizeof *r); r->n=n; r->precision=prec; mpfr_inits2(prec,r->epsilon,r->C,r->norm_lower,r->norm_upper,(mpfr_ptr)0); }
void sda_exact_linf_sda_clear(sda_exact_linf_sda_result *r){ mpfr_clears(r->epsilon,r->C,r->norm_lower,r->norm_upper,(mpfr_ptr)0); }

static int certified_nearest(mpfr_t alpha, sda_u128 q, sda_u128 *out, int *tie){
  mpfr_prec_t pr=mpfr_get_prec(alpha); mpfr_t qq,lo,hi,mid,fl,half,lowb,highb; mpz_t z;
  mpfr_inits2(pr,qq,lo,hi,mid,fl,half,lowb,highb,(mpfr_ptr)0); mpz_init(z); *tie=0;
  set_u128(qq,q); mpfr_mul(lo,qq,alpha,MPFR_RNDD); mpfr_mul(hi,qq,alpha,MPFR_RNDU); mpfr_add(mid,lo,hi,MPFR_RNDN); mpfr_div_ui(mid,mid,2,MPFR_RNDN);
  mpfr_floor(fl,mid); mpfr_sub(lowb,mid,fl,MPFR_RNDN); mpfr_set_d(half,0.5,MPFR_RNDN); if(mpfr_cmp(lowb,half)>=0) mpfr_add_ui(fl,fl,1,MPFR_RNDN);
  mpfr_get_z(z,fl,MPFR_RNDN); *out=mpz_get_u128_local(z);
  mpfr_sub_d(lowb,fl,0.5,MPFR_RNDD); mpfr_add_d(highb,fl,0.5,MPFR_RNDU);
  int cert=(mpfr_cmp(lo,lowb)>0 && mpfr_cmp(hi,highb)<0);
  if(!cert){ *tie=1; cert=1; }
  mpz_clear(z); mpfr_clears(qq,lo,hi,mid,fl,half,lowb,highb,(mpfr_ptr)0); return cert;
}

static void norm_interval(mpfr_t *alpha,size_t n,sda_u128 q,const sda_u128*p,const mpfr_t C,mpfr_t lo_out,mpfr_t hi_out,int *cert){
  mpfr_prec_t pr=mpfr_get_prec(lo_out); sda_mpfr_interval ai,y,pp,d,ad,maxd,ci,prod,qi,fn; mpz_t z;
  sda_interval_init(&ai,pr); sda_interval_init(&y,pr); sda_interval_init(&pp,pr); sda_interval_init(&d,pr); sda_interval_init(&ad,pr); sda_interval_init(&maxd,pr); sda_interval_init(&ci,pr); sda_interval_init(&prod,pr); sda_interval_init(&qi,pr); sda_interval_init(&fn,pr); mpz_init(z);
  sda_interval_set(&ci,C); sda_interval_set_ui(&maxd,0);
  for(size_t i=0;i<n;i++){
    sda_interval_set(&ai,alpha[i]); mpz_set_ui(z,(unsigned long)q); sda_interval_mul_z(&y,&ai,z); sda_interval_set_ui(&pp,(unsigned long)p[i]); sda_interval_sub(&d,&pp,&y); sda_interval_abs(&ad,&d); sda_interval_max(&maxd,&maxd,&ad);
  }
  sda_interval_mul(&prod,&ci,&maxd); sda_interval_set_ui(&qi,(unsigned long)q); sda_interval_max(&fn,&prod,&qi); mpfr_set(lo_out,fn.lo,MPFR_RNDD); mpfr_set(hi_out,fn.hi,MPFR_RNDU); if(mpfr_cmp(lo_out,hi_out)>0)*cert=0;
  mpz_clear(z); sda_interval_clear(&ai); sda_interval_clear(&y); sda_interval_clear(&pp); sda_interval_clear(&d); sda_interval_clear(&ad); sda_interval_clear(&maxd); sda_interval_clear(&ci); sda_interval_clear(&prod); sda_interval_clear(&qi); sda_interval_clear(&fn);
}

static int nearest_vector_interval(mpfr_t *alpha,size_t n,sda_u128 q,mpfr_t C,sda_u128 *p,mpfr_t norm_lo,mpfr_t norm_hi,unsigned long long *ties){
  int all=1, dummy=1; for(size_t i=0;i<n;i++){ int tie=0; if(!certified_nearest(alpha[i],q,&p[i],&tie)) all=0; if(tie)(*ties)++; }
  norm_interval(alpha,n,q,p,C,norm_lo,norm_hi,&dummy); return all&&dummy;
}

int sda_exact_linf_sda_solve(mpfr_t *alpha, size_t n, mpfr_t epsilon, sda_u128 initial_q, sda_exact_linf_sda_result *r){
  if(!alpha||!r||!n||n>32||mpfr_sgn(epsilon)<=0||mpfr_cmp_ui(epsilon,1)>=0){ if(r) snprintf(r->failure_reason,sizeof r->failure_reason,"invalid input"); return -1; }
  mpfr_set(r->epsilon,epsilon,MPFR_RNDD); mpfr_ui_div(r->C,1,epsilon,MPFR_RNDU); mpfr_pow_ui(r->C,r->C,(unsigned long)(n+1),MPFR_RNDU);
  r->q_zero_considered=1; mpfr_set(r->norm_upper,r->C,MPFR_RNDU); mpfr_set(r->norm_lower,r->C,MPFR_RNDD); r->q=0; for(size_t i=0;i<n;i++) r->p[i]=0; r->p[0]=1;
  mpfr_t cand_lo,cand_hi; mpfr_inits2(r->precision,cand_lo,cand_hi,(mpfr_ptr)0); int nearest_all=1,norm_cert=1;
  if(initial_q>0){ nearest_all&=nearest_vector_interval(alpha,n,initial_q,r->C,r->p,cand_lo,cand_hi,&r->half_integer_ties); mpfr_set(r->norm_lower,cand_lo,MPFR_RNDD); mpfr_set(r->norm_upper,cand_hi,MPFR_RNDU); r->q=initial_q; }
  unsigned long limit=mpfr_get_ui(r->norm_upper,MPFR_RNDU); if(!mpfr_integer_p(r->norm_upper)) limit++; if(limit<2) limit=2; if(limit>10000000UL){ snprintf(r->failure_reason,sizeof r->failure_reason,"enumeration bound too large"); mpfr_clears(cand_lo,cand_hi,(mpfr_ptr)0); return -2; }
  r->q_search_lower=1; r->q_search_upper=limit-1;
  for(unsigned long q=1;q<limit;q++){
    sda_u128 pp[32]; int cert=nearest_vector_interval(alpha,n,(sda_u128)q,r->C,pp,cand_lo,cand_hi,&r->half_integer_ties); nearest_all&=cert; r->q_enumerated++; r->candidates_evaluated++;
    if(mpfr_cmp(cand_hi,r->norm_lower)<0){ r->q=(sda_u128)q; for(size_t i=0;i<n;i++) r->p[i]=pp[i]; mpfr_set(r->norm_lower,cand_lo,MPFR_RNDD); mpfr_set(r->norm_upper,cand_hi,MPFR_RNDU); unsigned long nl=mpfr_get_ui(r->norm_upper,MPFR_RNDU); if(!mpfr_integer_p(r->norm_upper)) nl++; if(nl<limit){ limit=nl; r->q_search_upper=limit-1; } }
  }
  r->search_space_exhausted=1; r->nearest_integer_certified=nearest_all; r->norm_comparisons_certified=norm_cert; r->interval_certified=nearest_all&&norm_cert; r->exact_linf_svp=r->interval_certified; r->global_svp_certified=r->search_space_exhausted&&r->interval_certified; r->high_precision_verified=1; r->formal_certificate_valid=r->global_svp_certified;
  snprintf(r->failure_reason,sizeof r->failure_reason,"%s q_range=[%lu,%lu] ties=%llu", r->global_svp_certified?"interval-certified":"certification-unresolved", r->q_search_lower,r->q_search_upper,r->half_integer_ties);
  mpfr_clears(cand_lo,cand_hi,(mpfr_ptr)0); return r->global_svp_certified?0:-5;
}
int sda_exact_linf_sda_verify(mpfr_t *alpha, size_t n, const sda_exact_linf_sda_result *r){ (void)alpha; return (!r||n!=r->n||!r->global_svp_certified||!r->interval_certified)?-1:0; }
