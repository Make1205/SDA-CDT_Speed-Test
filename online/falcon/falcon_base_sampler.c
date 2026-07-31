#include "falcon_base_sampler.h"
#include "sdat_ref.h"

/* Falcon reference provenance: the Original path uses the gaussian0_sampler()
 * base distribution thresholds from the Falcon reference implementation
 * (sign.c, gaussian0_sampler). This file implements only the center-0,
 * sigma0=1.8205 half-Gaussian base sampler over support {0,...,18}; it does
 * not implement samplerZ, BerExp, FFT sampling, signing, verification, or keygen.
 * Random 72-bit words are interpreted little-endian as three 24-bit limbs
 * packed in the existing sdat_u72 representation. */

static int draw_u72(sdat_randombytes_fn randombytes, void *ctx, sdat_u72 *x, sdat_stats *stats, int rejected) {
    uint8_t b[FALCON_BASE_RANDOM_BYTES];
    if (!randombytes || !x) return -1;
    if (randombytes(ctx, b, sizeof b)) return -2;
    *x = sdat_u72_from_le9(b);
    if (stats) {
        stats->attempts++;
        stats->random_bytes += sizeof b;
        stats->random_bits += 72;
        if (rejected) stats->rejections++;
    }
    return 0;
}

int falcon_original_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out) {
    if (!out) return -1;
    *out = online_lookup_u72_reverse_tail(x, (const sdat_u72 *)original_cdt_table_falcon_base.thresholds,
                                          original_cdt_table_falcon_base.threshold_count);
    return *out <= FALCON_BASE_SUPPORT_MAX ? 0 : -2;
}

int falcon_sda_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out, int *accepted) {
    if (!out || !accepted) return -1;
    if (sdat_u72_ge(x, sda_table_falcon_base.denominator_u72)) {
        *accepted = 0;
        return 0;
    }
    *accepted = 1;
    *out = online_lookup_u72(x, (const sdat_u72 *)sda_table_falcon_base.thresholds,
                             sda_table_falcon_base.threshold_count);
    return *out <= FALCON_BASE_SUPPORT_MAX ? 0 : -2;
}

int falcon_original_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats) {
    sdat_u72 x;
    int rc = draw_u72(randombytes, ctx, &x, stats, 0);
    if (rc) return rc;
    return falcon_original_gaussian0_sample_from_u72(x, out);
}

size_t falcon_original_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats) {
    if (!out && n) return 0;
    if (!randombytes && n) return 0;
    if (!stats) {
        const sdat_u72 *thr = (const sdat_u72 *)original_cdt_table_falcon_base.thresholds;
        const size_t tn = original_cdt_table_falcon_base.threshold_count;
        size_t i = 0;
        for (; i < n; i++) {
            uint8_t b[FALCON_BASE_RANDOM_BYTES];
            if (randombytes(ctx, b, sizeof b)) return i;
            uint32_t y = online_lookup_u72_reverse_tail(sdat_u72_from_le9(b), thr, tn);
            if (y > FALCON_BASE_SUPPORT_MAX) return i;
            out[i] = y;
        }
        return i;
    }
    *stats = (sdat_stats){0};
    size_t i = 0;
    for (; i < n; i++) {
        if (falcon_original_gaussian0_sample(randombytes, ctx, &out[i], stats)) break;
    }
    return i;
}

int falcon_sda_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, sdat_stats *stats) {
    if (!out) return -1;
    for (;;) {
        sdat_u72 x;
        int rc = draw_u72(randombytes, ctx, &x, 0, 0);
        if (rc) return rc;
        int accepted = 0;
        rc = falcon_sda_gaussian0_sample_from_u72(x, out, &accepted);
        if (stats) {
            stats->attempts++;
            stats->random_bytes += FALCON_BASE_RANDOM_BYTES;
            stats->random_bits += 72;
            if (!accepted) stats->rejections++;
        }
        if (rc) return rc;
        if (accepted) return 0;
    }
}

size_t falcon_sda_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx, uint32_t *out, size_t n, sdat_stats *stats) {
    if (!out && n) return 0;
    if (!randombytes && n) return 0;
    if (!stats) {
        const sdat_u72 *thr = (const sdat_u72 *)sda_table_falcon_base.thresholds;
        const size_t tn = sda_table_falcon_base.threshold_count;
        const sdat_u72 q = sda_table_falcon_base.denominator_u72;
        size_t i = 0;
        for (; i < n; i++) {
            sdat_u72 x;
            do {
                uint8_t b[FALCON_BASE_RANDOM_BYTES];
                if (randombytes(ctx, b, sizeof b)) return i;
                x = sdat_u72_from_le9(b);
            } while (sdat_u72_ge(x, q));
            uint32_t y = 0;
            for (size_t j = 0; j < tn; j++) {
                y += (uint32_t)sdat_u72_ge(x, thr[j]);
            }
            if (y > FALCON_BASE_SUPPORT_MAX) return i;
            out[i] = y;
        }
        return i;
    }
    *stats = (sdat_stats){0};
    size_t i = 0;
    for (; i < n; i++) {
        if (falcon_sda_gaussian0_sample(randombytes, ctx, &out[i], stats)) break;
    }
    return i;
}

uint64_t falcon_base_checksum(const uint32_t *out, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; i++) {
        h ^= out[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static uint64_t falcon_stage_mark(falcon_stage_timing*t){return t&&t->read?t->read(t->context):0;}
static int falcon_stage_workspace_ok(const falcon_stage_workspace*w,size_t block){return w&&w->candidates&&block&&w->capacity>=block;}
static int falcon_original_stage_map(uint32_t*out,const sdat_u72*x,size_t n){const sdat_u72*thr=(const sdat_u72*)original_cdt_table_falcon_base.thresholds;size_t tn=original_cdt_table_falcon_base.threshold_count;for(size_t i=0;i<n;i++){out[i]=online_lookup_u72_reverse_tail(x[i],thr,tn);if(out[i]>FALCON_BASE_SUPPORT_MAX)return -1;}return 0;}
static int falcon_sda_stage_map(uint32_t*out,const sdat_u72*x,size_t n){const sdat_u72*thr=(const sdat_u72*)sda_table_falcon_base.thresholds;size_t tn=sda_table_falcon_base.threshold_count;for(size_t i=0;i<n;i++){out[i]=online_lookup_u72(x[i],thr,tn);if(out[i]>FALCON_BASE_SUPPORT_MAX)return -1;}return 0;}
int falcon_original_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block,falcon_stage_workspace*w,sdat_stats*st,falcon_stage_timing*tm){if((!out&&n)||(!raw&&n)||!falcon_stage_workspace_ok(w,block)||raw_len<n*FALCON_BASE_RANDOM_BYTES)return -1;if(st)*st=(sdat_stats){0};if(tm){tm->input_cycles=0;tm->mapping_cycles=0;tm->outer_cycles=0;}uint64_t outer0=falcon_stage_mark(tm);size_t done=0;while(done<n){size_t take=n-done<block?n-done:block;uint64_t t0=falcon_stage_mark(tm);for(size_t i=0;i<take;i++)w->candidates[i]=sdat_u72_from_le9(raw+(done+i)*FALCON_BASE_RANDOM_BYTES);uint64_t t1=falcon_stage_mark(tm);if(falcon_original_stage_map(out+done,w->candidates,take))return -2;uint64_t t2=falcon_stage_mark(tm);if(tm&&tm->read){tm->input_cycles+=t1-t0;tm->mapping_cycles+=t2-t1;}done+=take;}uint64_t outer1=falcon_stage_mark(tm);if(tm&&tm->read)tm->outer_cycles=outer1-outer0;if(st){st->attempts=n;st->random_bytes=n*FALCON_BASE_RANDOM_BYTES;st->random_bits=n*72;}return 0;}
int falcon_sda_block_staged_sample_n(uint32_t*out,size_t n,const uint8_t*raw,size_t raw_len,size_t block,falcon_stage_workspace*w,sdat_stats*st,falcon_stage_timing*tm){if((!out&&n)||(!raw&&n)||!falcon_stage_workspace_ok(w,block))return -1;if(st)*st=(sdat_stats){0};if(tm){tm->input_cycles=0;tm->mapping_cycles=0;tm->outer_cycles=0;}uint64_t outer0=falcon_stage_mark(tm);size_t done=0,pos=0;const sdat_u72 q=sda_table_falcon_base.denominator_u72;while(done<n){size_t take=n-done<block?n-done:block,accepted=0;uint64_t t0=falcon_stage_mark(tm);while(accepted<take){if(pos+FALCON_BASE_RANDOM_BYTES>raw_len)return -2;sdat_u72 x=sdat_u72_from_le9(raw+pos);pos+=FALCON_BASE_RANDOM_BYTES;if(sdat_u72_lt(x,q))w->candidates[accepted++]=x;}uint64_t t1=falcon_stage_mark(tm);if(falcon_sda_stage_map(out+done,w->candidates,take))return -3;uint64_t t2=falcon_stage_mark(tm);if(tm&&tm->read){tm->input_cycles+=t1-t0;tm->mapping_cycles+=t2-t1;}done+=take;}uint64_t outer1=falcon_stage_mark(tm);if(tm&&tm->read)tm->outer_cycles=outer1-outer0;if(st){st->attempts=pos/FALCON_BASE_RANDOM_BYTES;st->rejections=st->attempts-n;st->random_bytes=pos;st->random_bits=st->attempts*72;}return 0;}
