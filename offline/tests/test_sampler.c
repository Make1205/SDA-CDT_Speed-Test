#include "sda_generated_tables.h"
#include "sda_sampler.h"
int main(void){sda_bench_rng r; sda_bench_rng_init(&r,9); for(size_t i=0;i<sda_generated_tables_count;i++){int x; for(int j=0;j<1000;j++){ if(sda_sample_symmetric(&sda_generated_tables[i],sda_bench_random_bytes,&r,0,1,&x))return 1; if(x < -sda_generated_tables[i].support_max || x > sda_generated_tables[i].support_max)return 1; }} return 0;}
