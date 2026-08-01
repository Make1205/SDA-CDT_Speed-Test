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
static sdat_u72 sub72(sdat_u72 a, sdat_u72 b) {
    sdat_u72 r;
    r.lo = a.lo - b.lo;
    r.hi = (uint8_t)(a.hi - b.hi - (a.lo < b.lo));
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
    sdat_u72 x = { 0x9e3779b97f4a7c15ULL, 173 };
    for (size_t i = 0; i < 100000; i++) {
        x.lo = x.lo * 2862933555777941757ULL + 3037000493ULL;
        x.hi = (uint8_t)(x.hi * 101u + 7u);
        if (falcon_original_gaussian0_sample_from_u72(x, &out)) return 16;
        if (out != online_lookup_u72_reverse_tail(
                x, thr, original_cdt_table_falcon_base.threshold_count)) return 17;
    }
    return 0;
}

static int check_sda_boundaries(void) {
    const sdat_u72 *thr = (const sdat_u72 *)sda_table_falcon_base.thresholds;
    const sdat_u72 *p = (const sdat_u72 *)sda_table_falcon_base.pmf;
    const falcon_u72_limbs *tail = falcon_gaussian0_sda_reverse_tail_table();
    sdat_u72 q = sda_table_falcon_base.denominator_u72;
    uint32_t out = 99; int acc = -1;
    if (falcon_sda_gaussian0_sample_from_u72((sdat_u72){0,0}, &out, &acc) || !acc || out != 0) return 20;
    sdat_u72 qminus = sub1(q);
    if (falcon_sda_gaussian0_sample_from_u72(qminus, &out, &acc) || !acc || out != 18) return 21;
    if (falcon_sda_gaussian0_sample_from_u72(sda_table_falcon_base.denominator_u72, &out, &acc) || acc) return 22;
    if (falcon_sda_gaussian0_sample_from_u72((sdat_u72){UINT64_MAX,255}, &out, &acc) || acc) return 23;
    for (size_t i = 0; i < 18; i++) {
        sdat_u72 expected = sub72(q, thr[i]);
        sdat_u72 actual = falcon_u72_limbs_to_sdat(tail[i]);
        if (sdat_u72_cmp(expected, actual)) return 24;
        if (i && !sdat_u72_lt(actual, falcon_u72_limbs_to_sdat(tail[i-1]))) return 25;
        sdat_u72 below = sub1(actual), above = add1(actual);
        uint32_t at, lo, hi;
        at = falcon_gaussian0_reverse_tail_lookup(falcon_u72_limbs_from_sdat(actual), tail);
        lo = falcon_gaussian0_reverse_tail_lookup(falcon_u72_limbs_from_sdat(below), tail);
        hi = falcon_gaussian0_reverse_tail_lookup(falcon_u72_limbs_from_sdat(above), tail);
        if (lo != at + 1 || hi != at) return 29;
        if (i + 1 < 18 && sdat_u72_cmp(sub72(actual, falcon_u72_limbs_to_sdat(tail[i+1])), p[i+1])) return 34;
    }
    if (sdat_u72_cmp(sub72(q, falcon_u72_limbs_to_sdat(tail[0])), p[0])) return 38;
    if (sdat_u72_cmp(falcon_u72_limbs_to_sdat(tail[17]), p[18])) return 39;
    if (sdat_u72_cmp(falcon_u72_limbs_to_sdat(tail[18]), (sdat_u72){0,0})) return 35;
    /* The online input stage reflects the coordinate, so the new runtime
     * representation remains pointwise identical to the old cumulative map. */
    sdat_u72 x = { 0x123456789abcdef0ULL, 42 };
    for (size_t i = 0; i < 100000; i++) {
        x.lo = x.lo * 6364136223846793005ULL + 1442695040888963407ULL;
        x.hi = (uint8_t)(x.hi * 73u + 19u);
        if (!sdat_u72_lt(x, q)) continue;
        if (falcon_sda_gaussian0_sample_from_u72(x, &out, &acc) || !acc) return 36;
        if (out != falcon_sda_gaussian0_cumulative_lookup_for_test(x)) return 37;
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
        bytes_ctx o1 = {buf, sizeof buf, 0}, o2 = {buf, sizeof buf, 0}; sdat_stats ost = {0};
        if (falcon_original_gaussian0_sample_n(bytes_cb, &o1, a, n, 0) != n) return 49;
        if (falcon_original_gaussian0_sample_n(bytes_cb, &o2, b, n, &ost) != n) return 50;
        if (memcmp(a, b, n * sizeof a[0]) || o1.pos != o2.pos || ost.random_bytes != o2.pos || ost.rejections) return 51;
        bytes_ctx c1 = {buf, sizeof buf, 0}, c2 = {buf, sizeof buf, 0}; sdat_stats st = {0};
        if (falcon_sda_gaussian0_sample_n(bytes_cb, &c1, a, n, 0) != n) return 52;
        if (falcon_sda_gaussian0_sample_n(bytes_cb, &c2, b, n, &st) != n) return 53;
        if (memcmp(a, b, n * sizeof a[0])) return 54;
        if (c1.pos != c2.pos || st.random_bytes != c2.pos || st.random_bits != st.attempts * 72) return 55;
        if (st.rejections + n != st.attempts) return 56;
        if (n == 1024 && falcon_base_checksum(a, n) != 16610768450925691870ULL) return 57;
    }
    return 0;
}

static int check_batch(void) {
    uint8_t buf[9 * 20]; for (size_t i = 0; i < sizeof buf; i++) buf[i] = (uint8_t)(i * 17 + 3);
    uint32_t out[20]; bytes_ctx c = {buf, sizeof buf, 0}; sdat_stats st;
    if (falcon_original_gaussian0_sample_n(bytes_cb, &c, out, 20, &st) != 20) return 40;
    for (size_t i = 0; i < 20; i++) if (out[i] > 18) return 41;
    if (falcon_base_checksum(out, 20) != 11733010875229530213ULL) return 42;
    return 0;
}

static uint64_t stage_clock(void*ctx){uint64_t*v=ctx;*v+=19;return *v;}
static int check_block_staged(void){static uint8_t raw[9*20000];static uint32_t fused[7777],staged[7777],instrumented[7777];static falcon_u72_limbs candidates[8192];const size_t counts[]={1,4095,4096,4097,7777},blocks[]={8192,4096,4096,4096,257};for(unsigned seed=0;seed<4;seed++){for(size_t i=0;i<sizeof raw;i++)raw[i]=(uint8_t)(i*37u+seed*53u+11u);for(int kind=0;kind<2;kind++)for(size_t ci=0;ci<sizeof counts/sizeof counts[0];ci++){size_t n=counts[ci],block=blocks[ci];bytes_ctx c={raw,sizeof raw,0};sdat_stats fs={0},ss={0},is={0};size_t got=kind?falcon_sda_gaussian0_sample_n(bytes_cb,&c,fused,n,&fs):falcon_original_gaussian0_sample_n(bytes_cb,&c,fused,n,&fs);if(got!=n)return 60+kind;falcon_stage_workspace ws={candidates,8192};int rc=kind?falcon_sda_block_staged_sample_n(staged,n,raw,sizeof raw,block,&ws,&ss,0):falcon_original_block_staged_sample_n(staged,n,raw,sizeof raw,block,&ws,&ss,0);if(rc||memcmp(fused,staged,n*sizeof fused[0])||memcmp(&fs,&ss,sizeof fs))return 62+kind;uint64_t clock=0;falcon_stage_timing tm={stage_clock,&clock,0,0,0};rc=kind?falcon_sda_block_staged_sample_n(instrumented,n,raw,sizeof raw,block,&ws,&is,&tm):falcon_original_block_staged_sample_n(instrumented,n,raw,sizeof raw,block,&ws,&is,&tm);size_t nb=(n+block-1)/block;if(rc||memcmp(staged,instrumented,n*sizeof staged[0])||memcmp(&ss,&is,sizeof ss)||tm.input_cycles+tm.mapping_cycles!=38*nb||tm.outer_cycles!=19*(3*nb+1)||tm.outer_cycles<tm.input_cycles+tm.mapping_cycles)return 64+kind;for(size_t i=0;i<n;i++)if(staged[i]>FALCON_BASE_SUPPORT_MAX)return 66+kind;}}
falcon_stage_workspace ws={candidates,8192};if(falcon_original_block_staged_sample_n(staged,1,raw,sizeof raw,0,&ws,0,0)!=-1||falcon_sda_block_staged_sample_n(staged,1,raw,sizeof raw,0,&ws,0,0)!=-1)return 68;sdat_u72 q=sda_table_falcon_base.denominator_u72,qm=sub1(q);sdat_u72_to_le9(q,raw);sdat_u72_to_le9(qm,raw+9);sdat_stats st={0};if(falcon_sda_block_staged_sample_n(staged,1,raw,18,1,&ws,&st,0)||st.attempts!=2||st.rejections!=1||st.random_bytes!=18||staged[0]!=18)return 69;return 0;}
int main(void) {
    int r;
    if ((r = check_tables())) return r;
    if ((r = check_original_boundaries())) return r;
    if ((r = check_sda_boundaries())) return r;
    if ((r = check_byte_order_and_rejection())) return r;
    if ((r = check_batch())) return r;
    if ((r = check_no_stats_equivalence())) return r;
    if ((r = check_block_staged())) return r;
    puts("falcon base sampler tests passed");
    return 0;
}
