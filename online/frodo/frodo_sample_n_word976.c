#include "frodo_sample_n_fast.h"
#include "sdat_tables.h"
#include "frodo_word_core.h"

static inline uint16_t sign976(uint16_t mag, uint8_t sign) {
    return (uint16_t)(((uint16_t)(-(uint16_t)(sign & 1u)) ^ mag) + (sign & 1u));
}

static inline uint16_t ge976(uint16_t x) {
    const uint16_t *t = sda_table_frodo976.thresholds;
    return (uint16_t)((x >= t[0]) + (x >= t[1]) + (x >= t[2]) +
                      (x >= t[3]) + (x >= t[4]) + (x >= t[5]) +
                      (x >= t[6]) + (x >= t[7]) + (x >= t[8]));
}
int frodo976_sda_word_no_stats(uint16_t *out,size_t n,const uint16_t *w,size_t wc){
    const uint16_t *src=w,*end=w+wc;
    uint16_t *dst=out,*dst_end=out+n;
    while(dst<dst_end&&src<end){
        uint16_t z=*src++;
        uint16_t c=frodo976_word_candidate(z);
        uint16_t sample=sign976(ge976(c),frodo976_word_sign(z));
        unsigned accept=(unsigned)frodo976_word_accept(c);
        *dst=sample;
        dst+=accept;
    }
    return dst==dst_end?0:-2;
}
int frodo976_sda_word_accept_before_map(uint16_t *out,size_t n,const uint16_t*w,size_t wc){size_t a=0,p=0;while(a<n&&p<wc){uint16_t z=w[p++];uint16_t c=frodo976_word_candidate(z);if(frodo976_word_accept(c))out[a++]=sign976(ge976(c),frodo976_word_sign(z));}return a==n?0:-2;}
