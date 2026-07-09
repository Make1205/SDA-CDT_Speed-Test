#include "falcon_sda_sampler.h"
#include "sda_rng.h"
#include <stdint.h>
#include <string.h>
static const uint8_t p[19]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static const uint8_t c[19]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
int main(void){ sda_bench_rng rng; sda_bench_rng_init(&rng,123); sda_rng_stats st={0}; sda_u128 x; if(falcon_sda_uniform_bnd_72((((sda_u128)1)<<72)-1,sda_bench_random_bytes,&rng,&st,&x)||x>=((((sda_u128)1)<<72)-1)) return 1; sda_table t={"Falcon","falcon-test","unit",0,18,5,0,0,0,19,19,p,c,38,95}; char e[64]; if(falcon_sda_table_validate(&t,e,sizeof e)) return 2; int out=999; if(falcon_sda_base_sample(&t,sda_bench_random_bytes,&rng,&st,&out)) return 3; return (out>=-18 && out<=18)?0:4; }
