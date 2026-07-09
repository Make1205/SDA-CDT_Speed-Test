#ifndef SDAT_TYPES_H
#define SDAT_TYPES_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint64_t lo; uint8_t hi; } sdat_u72;
static inline int sdat_u72_cmp(sdat_u72 a, sdat_u72 b){ if(a.hi!=b.hi) return a.hi<b.hi?-1:1; if(a.lo!=b.lo) return a.lo<b.lo?-1:1; return 0; }
static inline int sdat_u72_lt(sdat_u72 a,sdat_u72 b){return sdat_u72_cmp(a,b)<0;}
static inline int sdat_u72_ge(sdat_u72 a,sdat_u72 b){return sdat_u72_cmp(a,b)>=0;}
static inline sdat_u72 sdat_u72_from_le9(const uint8_t in[9]){ sdat_u72 r={0,0}; for(int i=7;i>=0;i--) r.lo=(r.lo<<8)|in[i]; r.hi=in[8]; return r; }
static inline void sdat_u72_to_le9(sdat_u72 x,uint8_t out[9]){ for(int i=0;i<8;i++) out[i]=(uint8_t)(x.lo>>(8*i)); out[8]=x.hi; }
typedef int (*sdat_randombytes_fn)(void *ctx, uint8_t *out, size_t out_len);
typedef enum { SDAT_TYPE_U8=1, SDAT_TYPE_U16=2, SDAT_TYPE_U72=9 } sdat_value_type;
typedef struct { uint64_t attempts,rejections,random_bytes,random_bits; } sdat_stats;
typedef struct {
 const char *table_id,*parameter_set,*source_kind,*source_artifact,*source_hash;
 int support_min,support_max; size_t support_length; sdat_value_type value_type; unsigned denominator_bits,random_draw_bits;
 size_t mass_count,threshold_count; const void *pmf; const void *thresholds; int terminal_threshold_stored;
 const char *zero_semantics,*sign_semantics,*output_semantics; size_t native_table_bytes,fixed_packed_bits;
 sdat_u72 denominator_u72; uint64_t denominator_u64; int available,heuristic_bkz,exact_svp,global_svp_certified;
} sdat_table;
#endif
