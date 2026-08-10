#include "frodo_sample_n_fast.h"
#include "sdat_tables.h"
#include "frodo_word_core.h"

static inline uint16_t sign1344(uint16_t mag, uint8_t sign) {
    return (uint16_t)(((uint16_t)(-(uint16_t)(sign & 1u)) ^ mag) + (sign & 1u));
}

static inline uint16_t ge1344(uint8_t x) {
    const uint8_t *t = sda_table_frodo1344.thresholds;
    return (uint16_t)((x >= t[0]) + (x >= t[1]) + (x >= t[2]) + (x >= t[3]));
}

int frodo1344_sda_word_no_stats_branchless(uint16_t *out,size_t n,const uint16_t *w,size_t wc){
    const uint16_t *src=w,*end=w+wc;
    uint16_t *dst=out,*dst_end=out+n;
    while(dst<dst_end&&src<end){
        uint16_t z=*src++;
        uint8_t c=frodo1344_word_candidate(z);
        uint16_t sample=sign1344(ge1344(c),frodo1344_word_sign(z));
        unsigned accept=(unsigned)frodo1344_word_accept(c);
        *dst=sample;
        dst+=accept;
    }
    return dst==dst_end?0:-2;
}
int frodo1344_sda_word_accept_before_map(uint16_t *out,size_t n,const uint16_t*w,size_t wc){size_t a=0,p=0;while(a<n&&p<wc){uint16_t z=w[p++];uint8_t c=frodo1344_word_candidate(z);if(frodo1344_word_accept(c))out[a++]=sign1344(ge1344(c),frodo1344_word_sign(z));}return a==n?0:-2;}
int frodo1344_sda_map_materialized(uint16_t*out,const uint16_t*c,const uint8_t*s,size_t n){for(size_t i=0;i<n;i++)out[i]=sign1344(ge1344((uint8_t)c[i]),s[i]);return 0;}
