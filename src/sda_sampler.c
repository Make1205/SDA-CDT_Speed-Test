#include "sda_sampler.h"
int sda_sample_magnitude(const sda_table*t,sda_random_bytes_fn fn,void*ctx,sda_rng_stats*st,int*out){ sda_u128 u; if(sda_uniform_bounded(t->denominator,fn,ctx,st,&u))return-1; *out=t->support_min+sda_table_select_index(t,u); return 0; }
int sda_sample_symmetric(const sda_table*t,sda_random_bytes_fn fn,void*ctx,sda_rng_stats*st,int consume_zero_sign,int*out){ int m; if(sda_sample_magnitude(t,fn,ctx,st,&m))return-1; uint8_t b=0; if(m!=0||consume_zero_sign){ if(fn(ctx,&b,1)) return -1; if(st) st->raw_random_bits+=1; } *out=(m!=0 && (b&1))?-m:m; return 0; }
