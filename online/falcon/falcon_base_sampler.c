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
    if (stats) *stats = (sdat_stats){0};
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
    if (stats) *stats = (sdat_stats){0};
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
