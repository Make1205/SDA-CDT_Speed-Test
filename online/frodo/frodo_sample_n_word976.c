#include "frodo_sample_n_fast.h"
#include "sdat_tables.h"

static inline uint16_t sign976(uint16_t mag, uint8_t sign) {
    return (uint16_t)(((uint16_t)(-(uint16_t)(sign & 1u)) ^ mag) + (sign & 1u));
}

static inline uint16_t ge976(uint16_t x) {
    const uint16_t *t = sda_table_frodo976.thresholds;
    return (uint16_t)((x >= t[0]) + (x >= t[1]) + (x >= t[2]) +
                      (x >= t[3]) + (x >= t[4]) + (x >= t[5]) +
                      (x >= t[6]) + (x >= t[7]) + (x >= t[8]));
}
int frodo976_sda_word_no_stats(uint16_t *out, size_t n,
                                const uint16_t *w, size_t wc) {
    const uint16_t *src = w;
    const uint16_t *end = w + wc;
    uint16_t *dst = out;
    uint16_t *dst_end = out + n;
    while (dst < dst_end && src < end) {
        uint16_t z = *src++;
        uint16_t c = (uint16_t)(z & 0x1fffu);
        uint16_t sample = sign976(ge976(c), (uint8_t)((z >> 13) & 1u));
        unsigned accept = (unsigned)(c < 7442u);
        *dst = sample;
        dst += accept;
    }
    return dst == dst_end ? 0 : -2;
}
