#include "falcon_base_sampler.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
static uint64_t ticks(void){ unsigned aux; _mm_lfence(); uint64_t r=__rdtscp(&aux); _mm_lfence(); return r; }
#else
static uint64_t ticks(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return (uint64_t)ts.tv_sec*1000000000ull+ts.tv_nsec; }
#endif

typedef struct { uint64_t s; } rng;
static int rb(void *ctx, uint8_t *out, size_t n){ rng *r=(rng*)ctx; for(size_t i=0;i<n;i++){ r->s=r->s*6364136223846793005ULL+1442695040888963407ULL; out[i]=(uint8_t)(r->s>>56); } return 0; }
static size_t envsz(const char *n, size_t d){ char *s=getenv(n); return (s&&*s)?strtoull(s,0,10):d; }
static int cmpd(const void*a,const void*b){ double x=*(const double*)a,y=*(const double*)b; return (x>y)-(x<y); }
static double pct_sorted(const double *v,size_t n,double p){ if(!n)return 0; double pos=p*(double)(n-1); size_t i=(size_t)pos; if(i+1>=n)return v[n-1]; return v[i]+(v[i+1]-v[i])*(pos-(double)i); }
static uint64_t mix(uint64_t x){ x^=x>>33; x*=0xff51afd7ed558ccdULL; x^=x>>33; x*=0xc4ceb9fe1a85ec53ULL; x^=x>>33; return x; }
static sdat_u72 deterministic_u72(size_t i, uint64_t salt){ return (sdat_u72){mix((uint64_t)i ^ salt), (uint8_t)mix(((uint64_t)i<<1) ^ (salt<<1))}; }

typedef struct { uint64_t cyc, checksum; sdat_stats stats; int status; } row;
static sdat_u72 *make_inputs(const char *variant, size_t n){
    sdat_u72 *x=calloc(n?n:1,sizeof *x); if(!x)return 0;
    if(!strcmp(variant,"falcon_original_portable")){
        for(size_t i=0;i<n;i++) x[i]=deterministic_u72(i,0x1234u);
    } else {
        size_t got=0, ctr=0;
        while(got<n){ sdat_u72 v=deterministic_u72(ctr++,0x5678u); if(sdat_u72_lt(v,sda_table_falcon_base.denominator_u72)) x[got++]=v; }
    }
    return x;
}
static row run_mapping(const char *variant, const sdat_u72 *in, size_t n){
    uint32_t *out=calloc(n?n:1,sizeof *out); row r={0}; if(!out){r.status=-99;return r;} uint64_t t0=ticks();
    if(!strcmp(variant,"falcon_original_portable")){
        for(size_t i=0;i<n;i++) r.status|=falcon_original_gaussian0_sample_from_u72(in[i],&out[i]);
    } else {
        for(size_t i=0;i<n;i++){ int acc=0; r.status|=falcon_sda_gaussian0_sample_from_u72(in[i],&out[i],&acc); if(!acc)r.status|=16; }
    }
    uint64_t t1=ticks(); r.cyc=t1-t0; r.checksum=falcon_base_checksum(out,n); free(out); return r;
}
static row run_end_to_end(const char *variant, size_t n, unsigned rep){
    uint32_t *out=calloc(n?n:1,sizeof *out); row r={0}; if(!out){r.status=-99;return r;} rng g={0x12345678u + 0x9e3779b97f4a7c15ULL*(rep+1)}; uint64_t t0=ticks();
    if(!strcmp(variant,"falcon_original_portable")) { if(falcon_original_gaussian0_sample_n(rb,&g,out,n,&r.stats)!=n) r.status=-1; }
    else { if(falcon_sda_gaussian0_sample_n(rb,&g,out,n,&r.stats)!=n) r.status=-2; }
    uint64_t t1=ticks(); r.cyc=t1-t0; r.checksum=falcon_base_checksum(out,n); free(out); return r;
}
static void print_summary(const char *variant,const char*mode,size_t n,double *cps,size_t reps){ double sum=0,min=cps[0],max=cps[0]; for(size_t i=0;i<reps;i++){sum+=cps[i]; if(cps[i]<min)min=cps[i]; if(cps[i]>max)max=cps[i];} double mean=sum/(double)reps, var=0; for(size_t i=0;i<reps;i++){double d=cps[i]-mean;var+=d*d;} var/=reps; qsort(cps,reps,sizeof *cps,cmpd); printf("Falcon,%s,%s,summary,%zu,0,0,%.6f,0,0,0,0,0,0,ok,median=%.6f;p10=%.6f;p90=%.6f;cv=%.9f;min=%.6f;max=%.6f\n",variant,mode,n,mean,cps[reps/2],pct_sorted(cps,reps,0.10),pct_sorted(cps,reps,0.90),mean?sqrt(var)/mean:0.0,min,max); }
int main(void){ size_t n=envsz("FALCON_BENCH_SAMPLES",200000), warm=envsz("FALCON_BENCH_WARMUP",50000), reps=envsz("FALCON_BENCH_REPETITIONS",31); const char *vars[]={"falcon_original_portable","falcon_sda_portable"}; const char*modes[]={"mapping_only","end_to_end"}; puts("scheme,variant,mode,row_type,samples,repetition,cycles_total,cycles_per_sample,samples_per_second,attempts_per_sample,rejections_per_sample,source_bits_per_sample,source_bytes_per_sample,checksum,status,summary"); for(size_t v=0;v<2;v++){ sdat_u72 *inputs=make_inputs(vars[v],n); sdat_u72 *warm_inputs=make_inputs(vars[v],warm); if(!inputs||!warm_inputs)return 2; for(size_t m=0;m<2;m++){ if(!strcmp(modes[m],"mapping_only")) (void)run_mapping(vars[v],warm_inputs,warm); else (void)run_end_to_end(vars[v],warm,0); double *cps=calloc(reps,sizeof *cps); if(!cps)return 3; for(size_t r=0;r<reps;r++){ row x=!strcmp(modes[m],"mapping_only")?run_mapping(vars[v],inputs,n):run_end_to_end(vars[v],n,(unsigned)r); double cp=n?(double)x.cyc/(double)n:0; cps[r]=cp; printf("Falcon,%s,%s,timing,%zu,%zu,%llu,%.6f,%.6f,%.9f,%.9f,%.9f,%.9f,%llu,%d,\n",vars[v],modes[m],n,r,(unsigned long long)x.cyc,cp,cp?1.0/cp:0.0,n?(double)x.stats.attempts/n:0.0,n?(double)x.stats.rejections/n:0.0,n?(double)x.stats.random_bits/n:0.0,n?(double)x.stats.random_bytes/n:0.0,(unsigned long long)x.checksum,x.status); } print_summary(vars[v],modes[m],n,cps,reps); free(cps);} free(inputs); free(warm_inputs);} return 0; }
