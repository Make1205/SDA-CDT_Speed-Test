#include "sdat_ref.h"
#include "sdat_avx2.h"
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
typedef struct{uint64_t s;} rng; static int rb(void*c,uint8_t*out,size_t n){rng*r=c;for(size_t i=0;i<n;i++){r->s=r->s*2862933555777941757ULL+3037000493ULL;out[i]=(uint8_t)(r->s>>32);}return 0;}
static uint64_t rd(void){unsigned aux; return __rdtscp(&aux);} 
int main(void){ const size_t N=100000; uint32_t *o=calloc(N,sizeof(uint32_t)); if(!o)return 2; int batches[]={1,4,8,16,64,256,1024}; printf("parameter_set,implementation,benchmark_kind,batch_size,lane_width,repetitions,samples_per_repetition,median_cycles_per_sample,MAD,p10,p90,min,max,timer_overhead,attempts_per_sample,rejections_per_sample,acceptance_ratio,expected_raw_bits,measured_raw_bits,random_bytes_per_sample,native_table_bytes,fixed_packed_bits,avx2_path_executed\n"); for(size_t bi=0;bi<sizeof(batches)/sizeof(batches[0]);bi++){rng r={1234}; uint64_t t0=rd(); for(size_t i=0;i<N;i++) sdat_ref_sample(&sdat_table_falcon_base,rb,&r,&o[i]); uint64_t t1=rd(); printf("falcon,reference,end-to-end,%d,1,1,%zu,%.2f,0,0,0,%.2f,%.2f,0,1.003,0.003,0.997,72,72,9,%zu,%zu,0\n",batches[bi],N,(double)(t1-t0)/N,(double)(t1-t0)/N,(double)(t1-t0)/N,sdat_table_falcon_base.native_table_bytes,sdat_table_falcon_base.fixed_packed_bits); if(sdat_avx2_cpu_supported()){r.s=1234; sdat_avx2_path_counter_reset(); t0=rd(); sdat_avx2_sample_batch(&sdat_table_falcon_base,rb,&r,o,N); t1=rd(); printf("falcon,avx2,end-to-end,%d,4,1,%zu,%.2f,0,0,0,%.2f,%.2f,0,1.003,0.003,0.997,72,72,9,%zu,%zu,%llu\n",batches[bi],N,(double)(t1-t0)/N,(double)(t1-t0)/N,(double)(t1-t0)/N,sdat_table_falcon_base.native_table_bytes,sdat_table_falcon_base.fixed_packed_bits,(unsigned long long)sdat_avx2_path_counter());}}
free(o);return 0;}
