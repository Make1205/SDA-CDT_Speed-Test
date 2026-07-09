#include "sdat_avx2.h"
#include <immintrin.h>
extern void sdat_avx2_count(uint64_t n);
int sdat_avx2_cpu_supported(void){
#if defined(__GNUC__) && (defined(__x86_64__)||defined(__i386__))
 return __builtin_cpu_supports("avx2");
#else
 return 0;
#endif
}
static __m256i uge64(__m256i a,__m256i b){ const __m256i s=_mm256_set1_epi64x((long long)0x8000000000000000ULL); __m256i ax=_mm256_xor_si256(a,s), bx=_mm256_xor_si256(b,s); __m256i lt=_mm256_cmpgt_epi64(bx,ax); return _mm256_andnot_si256(lt,_mm256_set1_epi64x(-1)); }
void sdat_avx2_lookup_u72_batch(const sdat_u72*x,size_t n,const sdat_u72*t,size_t tn,uint32_t*out){ size_t i=0; for(;i+4<=n;i+=4){ __m256i lo=_mm256_set_epi64x((long long)x[i+3].lo,(long long)x[i+2].lo,(long long)x[i+1].lo,(long long)x[i].lo); __m256i hi=_mm256_set_epi64x(x[i+3].hi,x[i+2].hi,x[i+1].hi,x[i].hi); uint32_t acc[4]={0,0,0,0}; for(size_t j=0;j<tn;j++){ __m256i th=_mm256_set1_epi64x(t[j].hi); __m256i gt=_mm256_cmpgt_epi64(hi,th); __m256i eq=_mm256_cmpeq_epi64(hi,th); __m256i ge=_mm256_and_si256(eq,uge64(lo,_mm256_set1_epi64x((long long)t[j].lo))); __m256i m=_mm256_or_si256(gt,ge); uint64_t mm[4]; _mm256_storeu_si256((__m256i*)mm,m); acc[0]+=(uint32_t)(mm[0]>>63); acc[1]+=(uint32_t)(mm[1]>>63); acc[2]+=(uint32_t)(mm[2]>>63); acc[3]+=(uint32_t)(mm[3]>>63); } out[i]=acc[0]; out[i+1]=acc[1]; out[i+2]=acc[2]; out[i+3]=acc[3]; sdat_avx2_count(4); } for(;i<n;i++){uint32_t r=0;for(size_t j=0;j<tn;j++)r+=(uint32_t)sdat_u72_ge(x[i],t[j]);out[i]=r;} }

int sdat_avx2_sample_batch(const sdat_table*t,sdat_randombytes_fn fn,void*ctx,uint32_t*out,size_t n){ if(n==0)return 0; if(!t||!fn||!out||!t->available)return -1; if(t->value_type!=SDAT_TYPE_U72)return -2; size_t done=0; while(done<n){ sdat_u72 xs[4]; size_t m=(n-done>=4)?4:n-done; for(size_t k=0;k<m;k++){ uint8_t b[9]; do{ if(fn(ctx,b,9))return -3; xs[k]=sdat_u72_from_le9(b); }while(sdat_u72_ge(xs[k],t->denominator_u72)); } sdat_avx2_lookup_u72_batch(xs,m,(const sdat_u72*)t->thresholds,t->threshold_count,out+done); done+=m; } return 0; }
