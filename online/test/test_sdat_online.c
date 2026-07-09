#include "sdat_ref.h"
#include "sdat_avx2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct{uint64_t s;} rng;
static int rb(void*ctx,uint8_t*out,size_t n){rng*r=ctx;for(size_t i=0;i<n;i++){r->s=r->s*6364136223846793005ULL+1442695040888963407ULL;out[i]=(uint8_t)(r->s>>56);}return 0;}
static int failrb(void*ctx,uint8_t*out,size_t n){(void)ctx;(void)out;(void)n;return -9;}
static int exhaustive(const sdat_table*t,int sda){uint32_t h[32]={0}; if(t->value_type==SDAT_TYPE_U8){const uint8_t*p=t->pmf,*c=t->thresholds; for(uint32_t x=0;x<t->denominator_u64;x++) h[online_lookup_u8((uint8_t)x,c,t->threshold_count)]++; for(size_t i=0;i<t->mass_count;i++) if(h[i]!=p[i]) return 1;} else if(t->value_type==SDAT_TYPE_U16){const uint16_t*p=t->pmf,*c=t->thresholds; uint32_t end=sda?(uint32_t)t->denominator_u64:32768; for(uint32_t x=0;x<end;x++) h[online_lookup_u16((uint16_t)x,c,t->threshold_count)]++; for(size_t i=0;i<t->mass_count;i++) if(h[i]!=p[i]) return 2;} return 0;}
static int cross(const sdat_table*t,int sda,size_t n){uint32_t*a=calloc(n,sizeof*a),*b=calloc(n,sizeof*b); if(!a||!b)return 90; rng r1={7},r2={7}; sdat_stats s1={0},s2={0}; int r=sda?sda_cdt_ref_sample_batch(t,rb,&r1,a,n,&s1):original_cdt_ref_sample_batch(t,rb,&r1,a,n,&s1); if(r)return 91; online_avx2_stats_reset(); r=sda?sda_cdt_avx2_sample_batch(t,rb,&r2,b,n,&s2):original_cdt_avx2_sample_batch(t,rb,&r2,b,n,&s2); if(r)return 92; int rc=memcmp(a,b,n*sizeof*a)?93:0; if(!rc&&(s1.attempts!=s2.attempts||s1.rejections!=s2.rejections||s1.random_bits!=s2.random_bits))rc=94; free(a);free(b);return rc;}
int main(void){const sdat_table*orig[]={&original_cdt_table_frodo640,&original_cdt_table_frodo976,&original_cdt_table_frodo1344,&original_cdt_table_falcon_base}; const sdat_table*sda[]={&sda_table_frodo640,&sda_table_frodo976,&sda_table_frodo1344,&sda_table_falcon_base}; for(size_t i=0;i<4;i++){ if(online_table_validate(orig[i]))return 1; if(online_table_validate(sda[i]))return 2; }
 if(exhaustive(&original_cdt_table_frodo640,0)||exhaustive(&original_cdt_table_frodo976,0)||exhaustive(&original_cdt_table_frodo1344,0)) return 3; if(exhaustive(&sda_table_frodo640,1)||exhaustive(&sda_table_frodo976,1)||exhaustive(&sda_table_frodo1344,1)) return 4;
 uint8_t le[9]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f}; sdat_u72 u=sdat_u72_from_le9(le); if(u.hi!=0x7f||u.lo!=UINT64_MAX) return 5; const sdat_u72 *th=(const sdat_u72*)sda_table_falcon_base.thresholds; if(online_lookup_u72((sdat_u72){0,0},th,18)!=0) return 6; if(online_lookup_u72(th[0],th,18)!=1) return 7; if(online_lookup_u72((sdat_u72){10215721069833441391ULL,254},th,18)!=18) return 8; rng rf={1}; uint32_t x; if(sda_cdt_ref_sample(&sda_table_falcon_base,failrb,0,&x,0)!=-2)return 9;
 if(sdat_avx2_cpu_supported()){ for(size_t i=0;i<4;i++) if(cross(orig[i],0,1000000))return 20+(int)i; for(size_t i=0;i<4;i++) if(cross(sda[i],1,1000000))return 30+(int)i; }
 puts("online Original CDT and SDA_CDT tests passed"); return 0; }
