#include "sda_rounding.h"
#include <stdlib.h>
#include <gmp.h>
typedef struct{size_t i; mpfr_t frac;} item;
static int cmp(const void*a,const void*b){ const item*x=a,*y=b; int c=mpfr_cmp(y->frac,x->frac); if(c) return c; return (x->i>y->i)-(x->i<y->i); }
static void mpfr_set_u128(mpfr_t r,sda_u128 q){ mpfr_set_ui_2exp(r,(unsigned long)(q>>64),64,MPFR_RNDN); mpfr_add_ui(r,r,(unsigned long)q,MPFR_RNDN); }
static sda_u128 mpz_to_u128(const mpz_t z){ sda_u128 v=0; size_t n=0; unsigned char buf[16]={0}; mpz_export(buf,&n,-1,1,0,0,z); for(size_t i=0;i<n&&i<16;i++) v|=((sda_u128)buf[i])<<(8*i); return v; }
int sda_balanced_rounding(mpfr_t*alpha,size_t n,sda_u128 q,sda_u128*p){ item*it=calloc(n,sizeof*it); mpfr_t y,floorv,qq; mpfr_inits2(mpfr_get_prec(alpha[0]),y,floorv,qq,(mpfr_ptr)0); mpfr_set_u128(qq,q); sda_u128 sum=0; mpz_t z; mpz_init(z); for(size_t i=0;i<n;i++){ it[i].i=i; mpfr_init2(it[i].frac,mpfr_get_prec(alpha[0])); mpfr_mul(y,qq,alpha[i],MPFR_RNDN); mpfr_floor(floorv,y); mpfr_get_z(z,floorv,MPFR_RNDZ); p[i]=mpz_to_u128(z); mpfr_sub(it[i].frac,y,floorv,MPFR_RNDN); sum+=p[i]; } qsort(it,n,sizeof*it,cmp); sda_u128 r=q-sum; for(sda_u128 j=0;j<r&&j<n;j++) p[it[(size_t)j].i]++; for(size_t i=0;i<n;i++) mpfr_clear(it[i].frac); free(it); mpz_clear(z); mpfr_clears(y,floorv,qq,(mpfr_ptr)0); return 0; }
