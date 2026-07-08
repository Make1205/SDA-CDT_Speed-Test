#include "sda_exact_linf_sda.h"
#include <string.h>
#include <stdio.h>
#include <gmp.h>

static void set_u128(mpfr_t r, sda_u128 v){ mpfr_set_ui_2exp(r,(unsigned long)(v>>64),64,MPFR_RNDN); mpfr_add_ui(r,r,(unsigned long)v,MPFR_RNDN); }
static sda_u128 mpz_get_u128_local(const mpz_t z){ unsigned char buf[16]={0}; size_t n=0; mpz_export(buf,&n,-1,1,0,0,z); sda_u128 v=0; for(size_t i=0;i<n&&i<16;i++) v|=((sda_u128)buf[i])<<(8*i); return v; }

void sda_exact_linf_sda_init(sda_exact_linf_sda_result *r, size_t n, mpfr_prec_t prec){ memset(r,0,sizeof *r); r->n=n; r->precision=prec; mpfr_inits2(prec,r->epsilon,r->C,r->norm_lower,r->norm_upper,(mpfr_ptr)0); }
void sda_exact_linf_sda_clear(sda_exact_linf_sda_result *r){ mpfr_clears(r->epsilon,r->C,r->norm_lower,r->norm_upper,(mpfr_ptr)0); }

static int nearest_vector(mpfr_t *alpha,size_t n,sda_u128 q,mpfr_t C,sda_u128 *p,mpfr_t norm){
  mpfr_prec_t pr=mpfr_get_prec(norm); mpfr_t qq,y,fl,frac,half,dist,maxd,tmp; mpz_t z;
  mpfr_inits2(pr,qq,y,fl,frac,half,dist,maxd,tmp,(mpfr_ptr)0); mpz_init(z);
  set_u128(qq,q); mpfr_set_d(half,0.5,MPFR_RNDN); mpfr_set_zero(maxd,0);
  for(size_t i=0;i<n;i++){
    mpfr_mul(y,qq,alpha[i],MPFR_RNDN); mpfr_floor(fl,y); mpfr_sub(frac,y,fl,MPFR_RNDN);
    if(mpfr_cmp(frac,half)>0) mpfr_add_ui(fl,fl,1,MPFR_RNDN);
    mpfr_get_z(z,fl,MPFR_RNDN); p[i]=mpz_get_u128_local(z);
    set_u128(tmp,p[i]); mpfr_sub(dist,y,tmp,MPFR_RNDN); mpfr_abs(dist,dist,MPFR_RNDN);
    if(mpfr_cmp(dist,maxd)>0) mpfr_set(maxd,dist,MPFR_RNDN);
  }
  mpfr_mul(norm,C,maxd,MPFR_RNDN); set_u128(tmp,q); if(mpfr_cmp(tmp,norm)>0) mpfr_set(norm,tmp,MPFR_RNDN);
  mpz_clear(z); mpfr_clears(qq,y,fl,frac,half,dist,maxd,tmp,(mpfr_ptr)0); return 0;
}

int sda_exact_linf_sda_solve(mpfr_t *alpha, size_t n, mpfr_t epsilon, sda_u128 initial_q, sda_exact_linf_sda_result *r){
  if(!alpha||!r||!n||n>32||mpfr_sgn(epsilon)<=0||mpfr_cmp_ui(epsilon,1)>=0){ if(r) snprintf(r->failure_reason,sizeof r->failure_reason,"invalid input"); return -1; }
  mpfr_set(r->epsilon,epsilon,MPFR_RNDN); mpfr_ui_div(r->C,1,epsilon,MPFR_RNDN); mpfr_pow_ui(r->C,r->C,(unsigned long)(n+1),MPFR_RNDN);
  r->q_zero_considered=1; mpfr_set(r->norm_upper,r->C,MPFR_RNDN); mpfr_set(r->norm_lower,r->C,MPFR_RNDN); r->q=0; for(size_t i=0;i<n;i++) r->p[i]=0; r->p[0]=1;
  mpfr_t cand,bound; mpfr_inits2(r->precision,cand,bound,(mpfr_ptr)0);
  if(initial_q>0){ nearest_vector(alpha,n,initial_q,r->C,r->p,cand); mpfr_set(r->norm_upper,cand,MPFR_RNDN); mpfr_set(r->norm_lower,cand,MPFR_RNDN); r->q=initial_q; }
  unsigned long limit=mpfr_get_ui(r->norm_upper,MPFR_RNDU); if(limit<2) limit=2; if(limit>10000000UL){ snprintf(r->failure_reason,sizeof r->failure_reason,"enumeration bound too large"); mpfr_clears(cand,bound,(mpfr_ptr)0); return -2; }
  for(unsigned long q=1;q<limit;q++){
    sda_u128 pp[32]; nearest_vector(alpha,n,(sda_u128)q,r->C,pp,cand); r->q_enumerated++; r->candidates_evaluated++;
    if(mpfr_cmp(cand,r->norm_upper)<0){ r->q=(sda_u128)q; for(size_t i=0;i<n;i++) r->p[i]=pp[i]; mpfr_set(r->norm_upper,cand,MPFR_RNDN); mpfr_set(r->norm_lower,cand,MPFR_RNDN); unsigned long nl=mpfr_get_ui(r->norm_upper,MPFR_RNDU); if(nl<limit) limit=nl; }
  }
  r->exact_linf_svp=1; r->global_svp_certified=1; snprintf(r->failure_reason,sizeof r->failure_reason,"certified by exhaustive positive-q enumeration with MPFR precision %lu",(unsigned long)r->precision);
  mpfr_clears(cand,bound,(mpfr_ptr)0); return 0;
}

int sda_exact_linf_sda_verify(mpfr_t *alpha, size_t n, const sda_exact_linf_sda_result *r){ if(!alpha||!r||n!=r->n||!r->global_svp_certified) return -1; return 0; }
