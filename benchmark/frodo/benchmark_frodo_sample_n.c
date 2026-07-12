#include "frodo_sampler.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__x86_64__)||defined(__i386__)
#include <x86intrin.h>
static unsigned long long ticks(void){return __rdtsc();}
#else
static unsigned long long ticks(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (unsigned long long)ts.tv_sec*1000000000ull+ts.tv_nsec;}
#endif
static uint64_t rs=1;static uint32_t rnd(void){rs=rs*6364136223846793005ULL+1442695040888963407ULL;return (uint32_t)(rs>>32);}static void fill16(uint16_t*x,size_t n,uint64_t seed){rs=seed;for(size_t i=0;i<n;i++)x[i]=(uint16_t)rnd();}static void fill8(uint8_t*x,size_t n,uint64_t seed){rs=seed;for(size_t i=0;i<n;i++)x[i]=(uint8_t)rnd();}static uint64_t cksum(const uint16_t*x,size_t n){uint64_t s=1469598103934665603ULL;for(size_t i=0;i<n;i++){s^=x[i];s*=1099511628211ULL;}return s;}
static size_t envsz(const char*n,size_t d){char*s=getenv(n);return(s&&*s)?strtoull(s,0,10):d;}
static const char*envs(const char*n,const char*d){char*s=getenv(n);return(s&&*s)?s:d;}
#if defined(__GNUC__) || defined(__clang__)
static void bench_barrier(void){__asm__ __volatile__("" ::: "memory");}
#else
static void bench_barrier(void){}
#endif
static void emit(const frodo_sampler_params*p,frodo_sampler_kind kind,frodo_backend backend,frodo_frontend frontend,const char*mode,int rep,size_t n,unsigned long long cyc,uint64_t sum,const frodo_sampler_stats*fs,int status){double cps=n?((double)cyc/(double)n):0.0;const sdat_stats*st=fs?&fs->stats:0;double att=(st&&st->attempts)?(double)st->attempts/n:(kind==FRODO_SAMPLER_ORIGINAL_CDT?1.0:0.0);double rej=(st&&n)?(double)st->rejections/n:0.0;double logical=0,physical=0;if(kind==FRODO_SAMPLER_ORIGINAL_CDT){logical=16;physical=16;}else if(st&&n){logical=(double)st->random_bits/n;physical=(double)st->random_bytes*8.0/n;}printf("Frodo,%s,%s,%s,%s,full-sampler-core,%s,%s,%zu,%ld,%d,%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%llu,%s\n",p->name,frodo_sampler_kind_name(kind),frodo_backend_name(backend),frodo_frontend_name(frontend),mode,frodo_implementation_label(kind,backend,frontend),n,(long)getpid(),rep,cyc,cps,att,rej,logical,physical,(unsigned long long)sum,status?"error":"ok");}
static int timed_run(const frodo_sampler_params*p,frodo_sampler_kind kind,frodo_backend backend,frodo_frontend frontend,uint16_t*out,size_t n,const uint8_t*buf,size_t blen,const uint16_t*words,size_t wc,frodo_sampler_stats*fs,unsigned long long*cyc,uint64_t*sum){if(kind==FRODO_SAMPLER_ORIGINAL_CDT&&words&&wc>=n)memcpy(out,words,n*sizeof*out);bench_barrier();unsigned long long t0=ticks();int rc=frodo_sample_n_dispatch(kind,backend,frontend,p->id,out,n,buf,blen,words,wc,fs);bench_barrier();unsigned long long t1=ticks();*cyc=t1-t0;*sum=cksum(out,n);return rc;}
static void one(const frodo_sampler_params*p,size_t n,const char*mode,int rep,int emit_rows){size_t blen=n*8+4096,wc=n*8+4096;uint8_t*buf=malloc(blen);uint16_t*words=malloc(wc*2),*out=malloc(n*2);if(!buf||!words||!out)exit(2);fill8(buf,blen,1000+(uint64_t)rep+17u*(uint64_t)p->id);fill16(words,wc,2000+(uint64_t)rep+19u*(uint64_t)p->id);struct impl{frodo_sampler_kind k;frodo_backend b;frodo_frontend f;} impls[]={{FRODO_SAMPLER_ORIGINAL_CDT,FRODO_BACKEND_REFERENCE,FRODO_FRONTEND_ORIGINAL_WORD},{FRODO_SAMPLER_SDA_CDT,FRODO_BACKEND_REFERENCE,FRODO_FRONTEND_WORD_ORIENTED},{FRODO_SAMPLER_SDA_CDT,FRODO_BACKEND_REFERENCE,FRODO_FRONTEND_PACKED_BIT},{FRODO_SAMPLER_ORIGINAL_CDT,FRODO_BACKEND_AVX2,FRODO_FRONTEND_ORIGINAL_WORD},{FRODO_SAMPLER_SDA_CDT,FRODO_BACKEND_AVX2,FRODO_FRONTEND_WORD_ORIENTED},{FRODO_SAMPLER_SDA_CDT,FRODO_BACKEND_AVX2,FRODO_FRONTEND_PACKED_BIT}};size_t m=sizeof impls/sizeof impls[0];for(size_t step=0;step<m;step++){size_t ii=(step+(size_t)rep)%m;frodo_sampler_stats fs;unsigned long long cyc=0;uint64_t sum=0;int rc=timed_run(p,impls[ii].k,impls[ii].b,impls[ii].f,out,n,buf,blen,words,wc,0,&cyc,&sum);
        if(impls[ii].k==FRODO_SAMPLER_ORIGINAL_CDT)memcpy(out,words,n*sizeof*out);
        int metrics_rc=frodo_sample_n_dispatch(impls[ii].k,impls[ii].b,impls[ii].f,p->id,out,n,buf,blen,words,wc,&fs);
        if(!rc)rc=metrics_rc;
        if(emit_rows)emit(p,impls[ii].k,impls[ii].b,impls[ii].f,mode,rep,n,cyc,sum,&fs,rc);}free(buf);free(words);free(out);}
int main(void){size_t reps=envsz("FRODO_BENCH_REPETITIONS",31),warm=envsz("FRODO_BENCH_WARMUP",5),equal=envsz("FRODO_BENCH_SAMPLE_COUNT",1048576),native=envsz("FRODO_BENCH_NATIVE_BATCH",0);const char*mode=envs("FRODO_BENCH_MODE_LABEL",native?"native-batch":"equal-size");unsigned seed=(unsigned)envsz("FRODO_BENCH_ORDER_SEED",0);puts("scheme,parameter_set,sampler_kind,backend,frontend,component,mode,implementation,sample_count,process_id,repetition,cycles_total,cycles_per_output,attempts_per_output,rejections_per_output,logical_bits_per_output,physical_bits_per_output,checksum,status");for(size_t r=0;r<warm;r++){const frodo_sampler_params*p=frodo_get_sampler_params((frodo_param_id)((r+seed)%3));one(p,native?p->native_sample_count:equal,mode,-1,0);}for(size_t r=0;r<reps;r++)for(size_t step=0;step<3;step++){frodo_param_id id=(frodo_param_id)((step+r+seed)%3);const frodo_sampler_params*p=frodo_get_sampler_params(id);one(p,native?p->native_sample_count:equal,mode,(int)r,1);}return 0;}
