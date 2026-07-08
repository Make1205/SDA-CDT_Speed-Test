#include <mpfr.h>
#include "sda_rounding.h"
int main(void){mpfr_t a[3]; for(int i=0;i<3;i++)mpfr_init2(a[i],256); mpfr_set_d(a[0],0.2,0); mpfr_set_d(a[1],0.3,0); mpfr_set_d(a[2],0.5,0); sda_u128 p[3]; sda_balanced_rounding(a,3,10,p); int ok=(p[0]+p[1]+p[2]==10); for(int i=0;i<3;i++)mpfr_clear(a[i]); return ok?0:1;}
