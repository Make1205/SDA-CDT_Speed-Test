#include "falcon_base_sampler.h"
#include "sdat_ref.h"
#include <stdio.h>
#include <string.h>

typedef struct { const uint8_t *p; size_t n, pos; } bytes_ctx;
static int bytes_cb(void *ctx, uint8_t *out, size_t n) {
    bytes_ctx *c = (bytes_ctx *)ctx;
    if (c->pos + n > c->n) return -1;
    memcpy(out, c->p + c->pos, n);
    c->pos += n;
    return 0;
}

static sdat_u72 add72(sdat_u72 a, sdat_u72 b) {
    sdat_u72 r;
    r.lo = a.lo + b.lo;
    r.hi = (uint8_t)(a.hi + b.hi + (r.lo < a.lo));
    return r;
}
static sdat_u72 sub1(sdat_u72 x) { if (x.lo) x.lo--; else { x.lo = UINT64_MAX; x.hi--; } return x; }
static sdat_u72 add1(sdat_u72 x) { x.lo++; if (!x.lo) x.hi++; return x; }

static int check_tables(void) {
    if (online_table_validate(&original_cdt_table_falcon_base)) return 1;
    if (online_table_validate(&sda_table_falcon_base)) return 2;
    if (original_cdt_table_falcon_base.support_min != 0 || original_cdt_table_falcon_base.support_max != 18) return 3;
    if (sda_table_falcon_base.support_min != 0 || sda_table_falcon_base.support_max != 18) return 4;
    if (sda_table_falcon_base.denominator_u72.hi != 254 || sda_table_falcon_base.denominator_u72.lo != 10215721069833441392ULL) return 5;
    sdat_u72 sum = {0,0}; const sdat_u72 *p = (const sdat_u72 *)sda_table_falcon_base.pmf;
    const sdat_u72 *c = (const sdat_u72 *)sda_table_falcon_base.thresholds;
    for (size_t i = 0; i < sda_table_falcon_base.mass_count; i++) {
        sum = add72(sum, p[i]);
        if (sdat_u72_cmp(sum, c[i])) return 6;
        if (i && sdat_u72_lt(c[i], c[i-1])) return 7;
    }
    if (sdat_u72_cmp(sum, sda_table_falcon_base.denominator_u72)) return 8;
    return 0;
}

static int check_original_boundaries(void) {
    const sdat_u72 *thr = (const sdat_u72 *)original_cdt_table_falcon_base.thresholds;
    uint32_t out = 99;
    if (falcon_original_gaussian0_sample_from_u72((sdat_u72){0,0}, &out) || out != 18) return 10;
    if (falcon_original_gaussian0_sample_from_u72((sdat_u72){UINT64_MAX,255}, &out) || out != 0) return 11;
    for (size_t i = 0; i + 1 < original_cdt_table_falcon_base.threshold_count; i++) {
        if (falcon_original_gaussian0_sample_from_u72(thr[i], &out)) return 12;
        uint32_t expect = online_lookup_u72_reverse_tail(thr[i], thr, original_cdt_table_falcon_base.threshold_count);
        if (out != expect || out > 18) return 13;
        sdat_u72 below = sub1(thr[i]);
        if (falcon_original_gaussian0_sample_from_u72(below, &out)) return 14;
        expect = online_lookup_u72_reverse_tail(below, thr, original_cdt_table_falcon_base.threshold_count);
        if (out != expect || out > 18) return 15;
    }
    return 0;
}

static int check_sda_boundaries(void) {
    const sdat_u72 *thr = (const sdat_u72 *)sda_table_falcon_base.thresholds;
    uint32_t out = 99; int acc = -1;
    if (falcon_sda_gaussian0_sample_from_u72((sdat_u72){0,0}, &out, &acc) || !acc || out != 0) return 20;
    sdat_u72 qminus = sub1(sda_table_falcon_base.denominator_u72);
    if (falcon_sda_gaussian0_sample_from_u72(qminus, &out, &acc) || !acc || out != 18) return 21;
    if (falcon_sda_gaussian0_sample_from_u72(sda_table_falcon_base.denominator_u72, &out, &acc) || acc) return 22;
    if (falcon_sda_gaussian0_sample_from_u72((sdat_u72){UINT64_MAX,255}, &out, &acc) || acc) return 23;
    for (size_t i = 0; i < sda_table_falcon_base.threshold_count; i++) {
        if (falcon_sda_gaussian0_sample_from_u72(thr[i], &out, &acc)) return 24;
        if (acc && out != online_lookup_u72(thr[i], thr, sda_table_falcon_base.threshold_count)) return 25;
        if (sdat_u72_cmp(thr[i], sda_table_falcon_base.denominator_u72)) {
            sdat_u72 plus = add1(thr[i]);
            if (sdat_u72_lt(plus, sda_table_falcon_base.denominator_u72)) {
                if (falcon_sda_gaussian0_sample_from_u72(plus, &out, &acc) || !acc) return 26;
            }
        }
    }
    return 0;
}

static int check_byte_order_and_rejection(void) {
    uint8_t b[27] = {0};
    sdat_u72 q = sda_table_falcon_base.denominator_u72;
    sdat_u72 qminus = sub1(q);
    sdat_u72_to_le9(q, b);
    sdat_u72_to_le9((sdat_u72){UINT64_MAX,255}, b + 9);
    sdat_u72_to_le9(qminus, b + 18);
    bytes_ctx c = {b, sizeof b, 0}; uint32_t out = 99; sdat_stats st = {0};
    if (falcon_sda_gaussian0_sample(bytes_cb, &c, &out, &st)) return 30;
    if (st.attempts != 3 || st.rejections != 2 || st.random_bits != 216 || st.random_bytes != 27 || out != 18) return 31;
    uint8_t one[9] = {1,2,3,4,5,6,7,8,9}; sdat_u72 x = sdat_u72_from_le9(one); uint8_t back[9]; sdat_u72_to_le9(x, back);
    if (memcmp(one, back, 9)) return 32;
    bytes_ctx shortc = {one, 8, 0}; if (falcon_original_gaussian0_sample(bytes_cb, &shortc, &out, &st) == 0) return 33;
    return 0;
}


static int check_no_stats_equivalence(void) {
    size_t lens[] = {0,1,2,3,7,16,31,64,257,1024};
    uint8_t buf[9 * 4096]; for (size_t i = 0; i < sizeof buf; i++) buf[i] = (uint8_t)(i * 29 + 11);
    uint32_t a[1024], b[1024];
    for (size_t li = 0; li < sizeof(lens)/sizeof(lens[0]); li++) {
        size_t n = lens[li];
        bytes_ctx c1 = {buf, sizeof buf, 0}, c2 = {buf, sizeof buf, 0}; sdat_stats st = {0};
        if (falcon_sda_gaussian0_sample_n(bytes_cb, &c1, a, n, 0) != n) return 50;
        if (falcon_sda_gaussian0_sample_n(bytes_cb, &c2, b, n, &st) != n) return 51;
        if (memcmp(a, b, n * sizeof a[0])) return 52;
        if (c1.pos != c2.pos || st.random_bytes != c2.pos || st.random_bits != st.attempts * 72) return 53;
        if (st.rejections + n != st.attempts) return 54;
    }
    return 0;
}

static int check_batch(void) {
    uint8_t buf[9 * 20]; for (size_t i = 0; i < sizeof buf; i++) buf[i] = (uint8_t)(i * 17 + 3);
    uint32_t out[20]; bytes_ctx c = {buf, sizeof buf, 0}; sdat_stats st;
    if (falcon_original_gaussian0_sample_n(bytes_cb, &c, out, 20, &st) != 20) return 40;
    for (size_t i = 0; i < 20; i++) if (out[i] > 18) return 41;
    if (!falcon_base_checksum(out, 20)) return 42;
    return 0;
}

int main(void) {
    int r;
    if ((r = check_tables())) return r;
    if ((r = check_original_boundaries())) return r;
    if ((r = check_sda_boundaries())) return r;
    if ((r = check_byte_order_and_rejection())) return r;
    if ((r = check_batch())) return r;
    if ((r = check_no_stats_equivalence())) return r;
    puts("falcon base sampler tests passed");
    return 0;
}
