#include "sda_rng.h"
int main(void){sda_bench_rng r; sda_bench_rng_init(&r,7); sda_u128 qs[]={1,2,3,102,(((sda_u128)1)<<64)+123}; for(int i=0;i<5;i++){for(int j=0;j<1000;j++){sda_u128 x; if(sda_uniform_bounded(qs[i],sda_bench_random_bytes,&r,0,&x)||x>=qs[i])return 1;}} return 0;}
