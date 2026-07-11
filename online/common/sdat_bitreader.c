#include "sdat_bitreader.h"
void sdat_bitreader_init(sdat_bitreader*r,const uint8_t*b,size_t l){r->buf=b;r->len=l;r->pos=0;r->reservoir=0;r->available=0;r->bits_consumed=0;}
int sdat_bitreader_take(sdat_bitreader*r,unsigned bits,uint32_t*out){ if(!r||!out||bits>32)return -1; while(r->available<bits){ if(r->pos>=r->len)return -2; r->reservoir|=((uint64_t)r->buf[r->pos++])<<r->available; r->available+=8; } uint64_t mask=bits==32?0xffffffffULL:((1ULL<<bits)-1ULL); *out=(uint32_t)(r->reservoir&mask); r->reservoir>>=bits; r->available-=bits; r->bits_consumed+=bits; return 0; }
size_t sdat_bitreader_source_bytes_consumed(const sdat_bitreader*r){return r?r->pos:0;}
