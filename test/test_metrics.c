#include <mpfr.h>
#include "sda_metrics.h"
int main(void){mpfr_t a[2]; mpfr_init2(a[0],256); mpfr_init2(a[1],256); mpfr_set_d(a[0],0.5,0); mpfr_set_d(a[1],0.5,0); sda_u128 p[2]={1,1}; sda_metrics m; sda_metrics_init(&m,256); sda_compute_metrics(a,2,p,2,2,&m); int ok=mpfr_zero_p(m.sd_support); sda_metrics_clear(&m); mpfr_clear(a[0]); mpfr_clear(a[1]); return ok?0:1;}
