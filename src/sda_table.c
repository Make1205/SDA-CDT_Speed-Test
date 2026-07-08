#include "sda_table.h"
#include <stdio.h>
#include <limits.h>
int sda_build_cumulative(const sda_u128*p,size_t n,sda_u128*c,sda_u128*q){ if(!p||!c||!q||!n)return-1; sda_u128 s=0; for(size_t i=0;i<n;i++){ s+=p[i]; c[i]=s; } *q=s; return 0; }
sda_integer_width sda_table_width_for_q(sda_u128 q){ if(q<=UINT8_MAX)return SDA_WIDTH_U8; if(q<=UINT16_MAX)return SDA_WIDTH_U16; if(q<=UINT32_MAX)return SDA_WIDTH_U32; if(q<=UINT64_MAX)return SDA_WIDTH_U64; return SDA_WIDTH_U128; }
size_t sda_table_element_bytes_for_width(sda_integer_width w){ return ((size_t)w+7u)/8u; }
static sda_u128 load_width(const void*p,size_t i,sda_integer_width w){ switch(w){case SDA_WIDTH_U8:return ((const uint8_t*)p)[i];case SDA_WIDTH_U16:return ((const uint16_t*)p)[i];case SDA_WIDTH_U32:return ((const uint32_t*)p)[i];case SDA_WIDTH_U64:return ((const uint64_t*)p)[i];default:return ((const sda_u128*)p)[i];} }
sda_u128 sda_table_mass_at(const sda_table*t,size_t i){return load_width(t->masses,i,sda_table_width_for_q(t->denominator));}
sda_u128 sda_table_cumulative_at(const sda_table*t,size_t i){return load_width(t->cumulative,i,sda_table_width_for_q(t->denominator));}
int sda_validate_table(const sda_table*t,char*err,size_t errn){ if(!t||!t->masses||!t->cumulative||!t->table_length||!t->denominator){snprintf(err,errn,"invalid null table");return-1;} sda_u128 s=0,prev=0; for(size_t i=0;i<t->table_length;i++){ s+=sda_table_mass_at(t,i); sda_u128 ci=sda_table_cumulative_at(t,i); if(ci<prev){snprintf(err,errn,"cumulative not monotone at %zu",i);return-1;} prev=ci; if(ci!=s){snprintf(err,errn,"cumulative mismatch at %zu",i);return-1;} } if(s!=t->denominator){snprintf(err,errn,"sum != q");return-1;} if(sda_table_cumulative_at(t,t->table_length-1)!=t->denominator){snprintf(err,errn,"terminal threshold != q");return-1;} return 0; }
int sda_table_select_index(const sda_table*t,sda_u128 u){ size_t idx=t->table_length-1; unsigned found=0; for(size_t i=0;i<t->table_length;i++){ unsigned take=(!found && u<sda_table_cumulative_at(t,i)); idx=take?i:idx; found|=take; } return (int)idx; }
size_t sda_table_native_bytes(const sda_table*t){ return t?t->table_length*sda_table_element_bytes_for_width(sda_table_width_for_q(t->denominator)):0; }
