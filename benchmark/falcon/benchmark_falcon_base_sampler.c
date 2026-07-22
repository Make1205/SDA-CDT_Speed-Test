#include "falcon_base_sampler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
static uint64_t ticks(void){ unsigned aux; _mm_lfence(); uint64_t r=__rdtscp(&aux); _mm_lfence(); return r; }
#else
static uint64_t ticks(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return (uint64_t)ts.tv_sec*1000000000ull+ts.tv_nsec; }
#endif
#if defined(__GNUC__) || defined(__clang__)
static void barrier(void){__asm__ __volatile__("" ::: "memory");}
#else
static void barrier(void){}
#endif
typedef struct { const uint8_t *p; size_t n,pos; } bytes_ctx;
static int bytes_cb(void *ctx,uint8_t*out,size_t n){bytes_ctx*c=(bytes_ctx*)ctx;if(c->pos+n>c->n)return -1;memcpy(out,c->p+c->pos,n);c->pos+=n;return 0;}
static uint64_t rs=1;static uint32_t rnd(void){rs=rs*6364136223846793005ULL+1442695040888963407ULL;return(uint32_t)(rs>>32);}static void fill8(uint8_t*x,size_t n,uint64_t seed){rs=seed;for(size_t i=0;i<n;i++)x[i]=(uint8_t)rnd();}static size_t envsz(const char*n,size_t d){char*s=getenv(n);return(s&&*s)?strtoull(s,0,10):d;}static const char*envs(const char*n,const char*d){char*s=getenv(n);return(s&&*s)?s:d;}
static void emit(const char*kind,const char*frontend,const char*mode,const char*impl,size_t n,int rep,uint64_t cyc,const sdat_stats*st,uint64_t sum,int status){double cpo=n?(double)cyc/(double)n:0.0;double apo=(st&&n)?(double)st->attempts/(double)n:0.0;double rpo=(st&&n)?(double)st->rejections/(double)n:0.0;double phys=(st&&n)?(double)st->random_bytes/(double)n:0.0;printf("Falcon,base-gaussian0,%s,reference,%s,full-sampler-core,%s,%s,%zu,%ld,%d,%llu,%.6f,%.6f,%.6f,9.000000,%.6f,72,%llu,%s\n",kind,frontend,mode,impl,n,(long)getpid(),rep,(unsigned long long)cyc,cpo,apo,rpo,phys,(unsigned long long)sum,status?"error":"ok");}
static void run_one(const char*kind,const char*frontend,const char*impl,size_t n,int rep,const char*mode,int er){size_t blen=(n*12+1024)*FALCON_BASE_RANDOM_BYTES;uint8_t*buf=malloc(blen);uint32_t*out=calloc(n?n:1,sizeof*out);if(!buf||!out)exit(2);fill8(buf,blen,0xC0FFEEu+(uint64_t)rep*17u+kind[0]);bytes_ctx c={buf,blen,0};barrier();uint64_t t0=ticks();size_t got=!strcmp(kind,"original-cdt")?falcon_original_gaussian0_sample_n(bytes_cb,&c,out,n,0):falcon_sda_gaussian0_sample_n(bytes_cb,&c,out,n,0);barrier();uint64_t t1=ticks();bytes_ctx m={buf,blen,0};sdat_stats st={0};size_t mgot=!strcmp(kind,"original-cdt")?falcon_original_gaussian0_sample_n(bytes_cb,&m,out,n,&st):falcon_sda_gaussian0_sample_n(bytes_cb,&m,out,n,&st);int status=(got!=n)||(mgot!=n);if(er)emit(kind,frontend,mode,impl,n,rep,t1-t0,&st,falcon_base_checksum(out,n),status);free(buf);free(out);}
int main(void){size_t reps=envsz("FALCON_BENCH_REPETITIONS",31),warm=envsz("FALCON_BENCH_WARMUP",5),n=envsz("FALCON_BENCH_SAMPLE_COUNT",1048576);const char*mode=envs("FALCON_BENCH_MODE","equal-size");puts("scheme,parameter_set,sampler_kind,backend,frontend,component,mode,implementation,sample_count,process_id,repetition,cycles_total,cycles_per_output,attempts_per_output,rejections_per_output,source_bytes_per_attempt,physical_bytes_per_output,random_precision_bits,checksum,status");for(size_t r=0;r<warm;r++){run_one("original-cdt","falcon-prng72","original-reference",n,-1,mode,0);run_one("sda-cdt","falcon-sda72","sda-reference",n,-1,mode,0);}for(size_t r=0;r<reps;r++){run_one("original-cdt","falcon-prng72","original-reference",n,(int)r,mode,1);run_one("sda-cdt","falcon-sda72","sda-reference",n,(int)r,mode,1);}return 0;}
