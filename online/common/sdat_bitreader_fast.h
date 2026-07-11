#ifndef SDAT_BITREADER_FAST_H
#define SDAT_BITREADER_FAST_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    const uint8_t *ptr;
    const uint8_t *end;
    uint64_t reservoir;
    unsigned available;
    uint64_t bits_consumed;
    uint64_t bytes_loaded;
    uint64_t fast_refills;
    uint64_t tail_refills;
} sdat_bitreader_fast;

static inline void sdat_bitreader_fast_init(sdat_bitreader_fast *r,const uint8_t *buf,size_t len){
    r->ptr=buf; r->end=buf+len; r->reservoir=0; r->available=0; r->bits_consumed=0; r->bytes_loaded=0; r->fast_refills=0; r->tail_refills=0;
}
static inline int sdat_fast_refill64(sdat_bitreader_fast *r,unsigned need){
    if(r->available>=need) return 0;
    if(r->available==0 && (size_t)(r->end-r->ptr)>=8){ uint64_t w; memcpy(&w,r->ptr,8); r->ptr+=8; r->reservoir = w; r->available = 64; r->bytes_loaded += 8; r->fast_refills++; return 0; }
    while(r->available<need && r->ptr<r->end){ r->reservoir |= ((uint64_t)*r->ptr++) << r->available; r->available += 8; r->bytes_loaded++; r->tail_refills++; }
    return r->available>=need?0:-2;
}
#define SDAT_TAKE_CONST(NAME,BITS,MASK) \
static inline int NAME(sdat_bitreader_fast *r,uint32_t *out){ \
    if(sdat_fast_refill64(r,(BITS))) return -2; \
    *out=(uint32_t)(r->reservoir & (MASK)); \
    r->reservoir >>= (BITS); r->available -= (BITS); r->bits_consumed += (BITS); return 0; }
SDAT_TAKE_CONST(sdat_take_1,1,1u)
SDAT_TAKE_CONST(sdat_take_7,7,0x7fu)
SDAT_TAKE_CONST(sdat_take_13,13,0x1fffu)
SDAT_TAKE_CONST(sdat_take_14,14,0x3fffu)
#undef SDAT_TAKE_CONST
#endif
