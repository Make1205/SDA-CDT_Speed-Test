#include "sda_exact_linf_sda.h"
#include <mpfr.h>
#include <stdlib.h>
int main(void){
  for(int rep=0; rep<500; rep++){
    size_t n=(size_t)(1+(rep%5)); mpfr_t a[5],eps; for(size_t i=0;i<n;i++){ mpfr_init2(a[i],512); mpfr_set_ui(a[i],(unsigned)(i+1+rep%7),MPFR_RNDN); mpfr_div_ui(a[i],a[i],(unsigned)(10+n+rep%11),MPFR_RNDN); }
    mpfr_init2(eps,512); mpfr_set_d(eps,0.5,MPFR_RNDN); sda_exact_linf_sda_result r; sda_exact_linf_sda_init(&r,n,512); if(sda_exact_linf_sda_solve(a,n,eps,16,&r)) return 1; if(!r.exact_linf_svp||!r.global_svp_certified||r.q_enumerated==0) return 2; sda_exact_linf_sda_clear(&r); for(size_t i=0;i<n;i++) mpfr_clear(a[i]); mpfr_clear(eps);
  }
  return 0;
}
