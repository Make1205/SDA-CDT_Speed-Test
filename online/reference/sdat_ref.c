#include "sdat_ref.h"
static void addst(sdat_stats*st,unsigned bytes,unsigned bits,int rej){ if(st){st->attempts++;st->random_bytes+=bytes;st->random_bits+=bits;if(rej)st->rejections++;}}
uint32_t sdat_ref_lookup_u8(uint8_t x,const uint8_t*t,size_t n){uint32_t j=0;for(size_t i=0;i<n;i++)j+=(uint32_t)(x>=t[i]);return j;}
uint32_t sdat_ref_lookup_u16(uint16_t x,const uint16_t*t,size_t n){uint32_t j=0;for(size_t i=0;i<n;i++)j+=(uint32_t)(x>=t[i]);return j;}
uint32_t sdat_ref_lookup_u32(uint32_t x,const uint32_t*t,size_t n){uint32_t j=0;for(size_t i=0;i<n;i++)j+=(uint32_t)(x>=t[i]);return j;}
uint32_t sdat_ref_lookup_u64(uint64_t x,const uint64_t*t,size_t n){uint32_t j=0;for(size_t i=0;i<n;i++)j+=(uint32_t)(x>=t[i]);return j;}
uint32_t sdat_ref_lookup_u72(sdat_u72 x,const sdat_u72*t,size_t n){uint32_t j=0;for(size_t i=0;i<n;i++)j+=(uint32_t)sdat_u72_ge(x,t[i]);return j;}
int sdat_ref_uniform_u8(uint8_t q,unsigned bits,sdat_randombytes_fn fn,void*ctx,uint8_t*out,sdat_stats*st){ if(!fn||!out||bits>8)return -1; uint8_t mask=(uint8_t)((1u<<bits)-1u),b; do{ if(fn(ctx,&b,1))return -2; b&=mask; addst(st,1,bits,b>=q);}while(b>=q); *out=b; return 0;}
int sdat_ref_uniform_u16(uint16_t q,unsigned bits,sdat_randombytes_fn fn,void*ctx,uint16_t*out,sdat_stats*st){ if(!fn||!out||bits>16)return -1; uint8_t b[2]; uint16_t mask=(bits==16)?65535u:(uint16_t)((1u<<bits)-1u),x; do{ if(fn(ctx,b,2))return -2; x=(uint16_t)(b[0]|((uint16_t)b[1]<<8)); x&=mask; addst(st,2,bits,x>=q);}while(x>=q); *out=x; return 0;}
int sdat_ref_uniform_u72(sdat_u72 q,sdat_randombytes_fn fn,void*ctx,sdat_u72*out,sdat_stats*st){ if(!fn||!out)return -1; uint8_t b[9]; sdat_u72 x; do{ if(fn(ctx,b,9))return -2; x=sdat_u72_from_le9(b); addst(st,9,72,sdat_u72_ge(x,q)); }while(sdat_u72_ge(x,q)); *out=x; return 0;}
int sdat_ref_sample(const sdat_table*t,sdat_randombytes_fn fn,void*ctx,uint32_t*out){ if(!t||!out||!t->available)return -1; if(t->value_type==SDAT_TYPE_U72){sdat_u72 x; int r=sdat_ref_uniform_u72(t->denominator_u72,fn,ctx,&x,0); if(r)return r; *out=sdat_ref_lookup_u72(x,(const sdat_u72*)t->thresholds,t->threshold_count); return 0;} return -3; }
int sdat_ref_sample_batch(const sdat_table*t,sdat_randombytes_fn fn,void*ctx,uint32_t*out,size_t n){ if(n==0)return 0; if(!out)return -1; for(size_t i=0;i<n;i++){int r=sdat_ref_sample(t,fn,ctx,&out[i]); if(r)return r;} return 0;}
