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
static double pct(double *v,size_t n,double p){ if(!n)return 0; double pos=p*(double)(n-1); size_t i=(size_t)pos; if(i+1>=n)return v[n-1]; return v[i]+(v[i+1]-v[i])*(pos-(double)i); }

typedef struct { uint64_t cyc, checksum; sdat_stats stats; int status; } row;
static row run_one(const char *variant, const char *mode, size_t n, unsigned rep){
    uint32_t *out=calloc(n?n:1,sizeof *out); row r={0}; if(!out){r.status=-99;return r;} rng g={0x12345678u + 0x9e3779b97f4a7c15ULL*(rep+1)}; uint64_t t0=ticks();
    if(!strcmp(variant,"falcon_original_portable")){
        if(!strcmp(mode,"mapping_only")){
            for(size_t i=0;i<n;i++){ uint8_t b[9]; rb(&g,b,9); sdat_u72 x=sdat_u72_from_le9(b); r.status|=falcon_original_gaussian0_sample_from_u72(x,&out[i]); r.stats.attempts++; r.stats.random_bits+=72; r.stats.random_bytes+=9; }
        } else if(falcon_original_gaussian0_sample_n(rb,&g,out,n,&r.stats)!=n) r.status=-1;
    } else {
        if(!strcmp(mode,"mapping_only")){
            for(size_t i=0;i<n;i++){ uint8_t b[9]; int acc=0; do{ rb(&g,b,9); sdat_u72 x=sdat_u72_from_le9(b); r.stats.attempts++; r.stats.random_bits+=72; r.stats.random_bytes+=9; r.status|=falcon_sda_gaussian0_sample_from_u72(x,&out[i],&acc); if(!acc)r.stats.rejections++; }while(!acc); }
        } else if(falcon_sda_gaussian0_sample_n(rb,&g,out,n,&r.stats)!=n) r.status=-2;
    }
    uint64_t t1=ticks(); r.cyc=t1-t0; r.checksum=falcon_base_checksum(out,n); free(out); return r;
}
static void print_summary(const char *variant,const char*mode,size_t n,double *cps,size_t reps){ double sum=0; for(size_t i=0;i<reps;i++)sum+=cps[i]; double mean=sum/(double)reps, var=0; for(size_t i=0;i<reps;i++){double d=cps[i]-mean;var+=d*d;} var/=reps; qsort(cps,reps,sizeof *cps,cmpd); printf("Falcon,%s,%s,summary,%zu,0,0,%.6f,0,0,0,0,0,0,median=%.6f;p10=%.6f;p90=%.6f;cv=%.9f\n",variant,mode,n,mean,cps[reps/2],pct(cps,reps,0.10),pct(cps,reps,0.90),mean?sqrt(var)/mean:0.0); }
int main(void){ size_t n=envsz("FALCON_BENCH_SAMPLES",200000), warm=envsz("FALCON_BENCH_WARMUP",10000), reps=envsz("FALCON_BENCH_REPETITIONS",21); const char *vars[]={"falcon_original_portable","falcon_sda_portable"}; const char*modes[]={"mapping_only","end_to_end"}; puts("scheme,variant,mode,row_type,samples,repetition,cycles_total,cycles_per_sample,samples_per_second,attempts_per_sample,rejections_per_sample,source_bits_per_sample,source_bytes_per_sample,checksum,status,summary"); for(size_t v=0;v<2;v++)for(size_t m=0;m<2;m++){ (void)run_one(vars[v],modes[m],warm,0); double *cps=calloc(reps,sizeof *cps); if(!cps)return 2; for(size_t r=0;r<reps;r++){ row x=run_one(vars[v],modes[m],n,(unsigned)r); double cp=n?(double)x.cyc/(double)n:0; cps[r]=cp; printf("Falcon,%s,%s,timing,%zu,%zu,%llu,%.6f,%.6f,%.9f,%.9f,%.9f,%.9f,%llu,%d,\n",vars[v],modes[m],n,r,(unsigned long long)x.cyc,cp,cp?1.0/cp:0.0,n?(double)x.stats.attempts/n:0.0,n?(double)x.stats.rejections/n:0.0,n?(double)x.stats.random_bits/n:0.0,n?(double)x.stats.random_bytes/n:0.0,(unsigned long long)x.checksum,x.status); } print_summary(vars[v],modes[m],n,cps,reps); free(cps);} return 0; }
