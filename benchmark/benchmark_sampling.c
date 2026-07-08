#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "sda_generated_tables.h"
#include "sda_sampler.h"
static double now(void){struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return ts.tv_sec+ts.tv_nsec*1e-9;}
int main(int argc,char**argv){(void)argc;(void)argv; printf("benchmark-only RNG; monotonic clock; repetitions=5 warmup=1000 iterations=200000\n"); for(size_t i=0;i<sda_generated_tables_count;i++){ const sda_table*t=&sda_generated_tables[i]; double best=1e99,worst=0,sum=0; volatile long sink=0; sda_rng_stats st={0}; for(int r=0;r<5;r++){ sda_bench_rng rng; sda_bench_rng_init(&rng,1234+r); int x; for(int w=0;w<1000;w++) sda_sample_symmetric(t,sda_bench_random_bytes,&rng,&st,1,&x); double a=now(); for(int k=0;k<200000;k++){sda_sample_symmetric(t,sda_bench_random_bytes,&rng,&st,1,&x); sink+=x;} double ns=(now()-a)*1e9/200000.0; if(ns<best)best=ns; if(ns>worst)worst=ns; sum+=ns; } char q[64]; sda_print_u128(t->denominator,q,sizeof q); printf("%s q=%s median_approx_ns=%.2f min=%.2f max=%.2f attempts_per_sample=%.4f ideal_packed_bits=%zu native_bytes=%zu sink=%ld\n",t->parameter_set,q,sum/5,best,worst,(double)st.attempts/(double)st.accepted_samples,t->ideal_packed_bits,t->native_storage_bytes,(long)sink); } return 0; }
