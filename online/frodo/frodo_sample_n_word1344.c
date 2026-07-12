#include "frodo_sample_n_fast.h"
#include "sdat_tables.h"

static inline uint16_t sign1344(uint16_t mag, uint8_t sign) {
    return (uint16_t)(((uint16_t)(-(uint16_t)(sign & 1u)) ^ mag) + (sign & 1u));
}

static inline uint16_t ge1344(uint8_t x) {
    const uint8_t *t = sda_table_frodo1344.thresholds;
    return (uint16_t)((x >= t[0]) + (x >= t[1]) + (x >= t[2]) + (x >= t[3]));
}

int frodo1344_sda_word_no_stats_branchless(uint16_t *out, size_t n,
                                            const uint16_t *w, size_t wc) {
    const uint16_t *src = w;
    const uint16_t *end = w + wc;
    uint16_t *dst = out;
    uint16_t *dst_end = out + n;
    while (dst < dst_end && src < end) {
        uint16_t z = *src++;
        uint8_t c = (uint8_t)(z & 0x7fu);
        uint16_t sample = sign1344(ge1344(c), (uint8_t)((z >> 7) & 1u));
        unsigned accept = (unsigned)(c < 102u);
        *dst = sample;
        dst += accept;
    }
    return dst == dst_end ? 0 : -2;
}
