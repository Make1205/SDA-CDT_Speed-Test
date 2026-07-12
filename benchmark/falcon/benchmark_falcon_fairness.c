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
static uint64_t rs=1;
static uint32_t rnd(void){rs=rs*6364136223846793005ULL+1442695040888963407ULL;return(uint32_t)(rs>>32);}
static void fill8(uint8_t*x,size_t n,uint64_t seed){rs=seed;for(size_t i=0;i<n;i++)x[i]=(uint8_t)rnd();}
static size_t envsz(const char*n,size_t d){char*s=getenv(n);return(s&&*s)?strtoull(s,0,10):d;}
static const char*envs(const char*n,const char*d){char*s=getenv(n);return(s&&*s)?s:d;}

typedef enum { PATH_ORIGINAL=0, PATH_SDA_OLD=1, PATH_SDA_NEW=2 } path_id;
typedef struct { uint64_t cycles; size_t got,pos; sdat_stats stats; uint64_t checksum; int status; } result;

static size_t sda_old_generic_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats) {
    if (!out && n) return 0;
    if (!randombytes && n) return 0;
    if (stats) *stats = (sdat_stats){0};
    size_t i = 0;
    for (; i < n; i++) {
        for (;;) {
            uint8_t b[FALCON_BASE_RANDOM_BYTES];
            if (randombytes(ctx, b, sizeof b)) return i;
            sdat_u72 x = sdat_u72_from_le9(b);
            int accepted = 0;
            uint32_t y = 0;
            int rc = falcon_sda_gaussian0_sample_from_u72(x, &y, &accepted);
            if (stats) {
                stats->attempts++;
                stats->random_bytes += FALCON_BASE_RANDOM_BYTES;
                stats->random_bits += 72;
                if (!accepted) stats->rejections++;
            }
            if (rc) return i;
            if (accepted) { out[i] = y; break; }
        }
    }
    return i;
}

static size_t run_path(path_id p, bytes_ctx *ctx, uint32_t *out, size_t n, sdat_stats *stats) {
    switch (p) {
    case PATH_ORIGINAL: return falcon_original_gaussian0_sample_n(bytes_cb, ctx, out, n, stats);
    case PATH_SDA_OLD: return sda_old_generic_sample_n(bytes_cb, ctx, out, n, stats);
    case PATH_SDA_NEW: return falcon_sda_gaussian0_sample_n(bytes_cb, ctx, out, n, stats);
    }
    return 0;
}
static const char *path_name(path_id p){return p==PATH_ORIGINAL?"original-current":(p==PATH_SDA_OLD?"sda-old-generic":"sda-new-batch");}
static const char *sampler_kind(path_id p){return p==PATH_ORIGINAL?"original-cdt":"sda-cdt";}

static result measure(path_id p,const uint8_t*b,size_t blen,uint32_t*out,size_t n){
    bytes_ctx c={b,blen,0};
    barrier(); uint64_t t0=ticks(); size_t got=run_path(p,&c,out,n,0); barrier(); uint64_t t1=ticks();
    bytes_ctx m={b,blen,0}; sdat_stats st={0}; uint32_t *tmp=calloc(n?n:1,sizeof*tmp); if(!tmp) exit(2);
    size_t mgot=run_path(p,&m,tmp,n,&st); uint64_t sum=falcon_base_checksum(tmp,n); free(tmp);
    result r={t1-t0,got,c.pos,st,sum,(got!=n)||(mgot!=n)};
    return r;
}

static int verify_sda_equiv(const uint8_t*b,size_t blen,size_t n,uint64_t *sum_old,uint64_t *sum_new,size_t *pos_old,size_t *pos_new,sdat_stats *st_old,sdat_stats *st_new){
    uint32_t *a=calloc(n?n:1,sizeof*a),*c=calloc(n?n:1,sizeof*c); if(!a||!c) exit(2);
    bytes_ctx ca={b,blen,0}, cc={b,blen,0};
    size_t ga=sda_old_generic_sample_n(bytes_cb,&ca,a,n,st_old);
    size_t gc=falcon_sda_gaussian0_sample_n(bytes_cb,&cc,c,n,st_new);
    *sum_old=falcon_base_checksum(a,n); *sum_new=falcon_base_checksum(c,n); *pos_old=ca.pos; *pos_new=cc.pos;
    int bad=(ga!=n)||(gc!=n)||(ca.pos!=cc.pos)||memcmp(a,c,n*sizeof*a)||st_old->attempts!=st_new->attempts||st_old->rejections!=st_new->rejections||st_old->random_bytes!=st_new->random_bytes||st_old->random_bits!=st_new->random_bits;
    free(a); free(c); return bad;
}

static void emit(const char*mode,int rep,int slot,path_id p,result r,size_t n,const char*order,int equiv_status){
    double cpo=n?(double)r.cycles/(double)n:0.0, apo=n?(double)r.stats.attempts/(double)n:0.0, rpo=n?(double)r.stats.rejections/(double)n:0.0, phys=n?(double)r.stats.random_bytes/(double)n:0.0;
    printf("Falcon,base-gaussian0,%s,reference,falcon-prng72,fairness,%s,%s,%zu,%ld,%d,%d,%s,%llu,%.6f,%.6f,%.6f,9.000000,%.6f,72,%zu,%zu,%llu,%s,%s\n",sampler_kind(p),mode,path_name(p),n,(long)getpid(),rep,slot,order,(unsigned long long)r.cycles,cpo,apo,rpo,phys,r.got,r.pos,(unsigned long long)r.checksum,(r.status||equiv_status)?"error":"ok",equiv_status?"sda_equivalence_failed":"ok");
}

int main(void){
    size_t reps=envsz("FALCON_BENCH_REPETITIONS",31),warm=envsz("FALCON_BENCH_WARMUP",5),n=envsz("FALCON_BENCH_SAMPLE_COUNT",1048576); const char*mode=envs("FALCON_BENCH_MODE","equal-size");
    const path_id orders[3][3]={{PATH_ORIGINAL,PATH_SDA_OLD,PATH_SDA_NEW},{PATH_SDA_NEW,PATH_SDA_OLD,PATH_ORIGINAL},{PATH_SDA_OLD,PATH_ORIGINAL,PATH_SDA_NEW}};
    const char *order_names[3]={"original-current>sda-old-generic>sda-new-batch","sda-new-batch>sda-old-generic>original-current","sda-old-generic>original-current>sda-new-batch"};
    puts("scheme,parameter_set,sampler_kind,backend,frontend,component,mode,implementation,sample_count,process_id,repetition,order_slot,order_pattern,cycles_total,cycles_per_output,attempts_per_output,rejections_per_output,source_bytes_per_attempt,physical_bytes_per_output,random_precision_bits,accepted_outputs,source_bytes,checksum,status,equivalence_status");
    size_t blen=(n*12+4096)*FALCON_BASE_RANDOM_BYTES; uint8_t *buf=malloc(blen); uint32_t *out=calloc(n?n:1,sizeof*out); if(!buf||!out) return 2;
    for(size_t r=0;r<warm+reps;r++){
        int emit_row=r>=warm; int rep=emit_row?(int)(r-warm):-1; int oi=(int)(r%3); fill8(buf,blen,0xFA17u+(uint64_t)rep*1009u+(uint64_t)oi*17u);
        uint64_t so=0,sn=0; size_t po=0,pn=0; sdat_stats sto={0},stn={0}; int equiv=verify_sda_equiv(buf,blen,n,&so,&sn,&po,&pn,&sto,&stn);
        (void)so;(void)sn;(void)po;(void)pn;
        for(int slot=0;slot<3;slot++){ result rr=measure(orders[oi][slot],buf,blen,out,n); if(emit_row) emit(mode,rep,slot,orders[oi][slot],rr,n,order_names[oi],equiv); }
    }
    free(buf); free(out); return 0;
}
