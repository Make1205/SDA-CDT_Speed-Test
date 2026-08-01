#include "falcon_base_sampler.h"
#include "sdat_ref.h"

/* This implements only Falcon's center-0, sigma0=1.8205 nonnegative
 * Gaussian0 base sampler over {0,...,18}.  It does not implement samplerZ,
 * BerExp, FFT sampling, signing, verification, or key generation. */

static int draw_u72(sdat_randombytes_fn randombytes, void *ctx, sdat_u72 *x,
                    sdat_stats *stats, int rejected) {
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

static uint32_t map_original(falcon_u72_limbs x) {
    return falcon_gaussian0_reverse_tail_lookup(
        x, falcon_gaussian0_original_reverse_tail_table());
}

static uint32_t map_sda(falcon_u72_limbs x) {
    return falcon_gaussian0_reverse_tail_lookup(
        x, falcon_gaussian0_sda_reverse_tail_table());
}

/* Reverse-tail coordinates run in the opposite direction from the retained
 * canonical cumulative table.  Reflecting an accepted uniform value through
 * q-1 is a bijection, preserves the exact raw-input KAT, and belongs to SDA's
 * input stage rather than to the shared mapping kernel. */
static falcon_u72_limbs sda_reverse_coordinate(falcon_u72_limbs x) {
    falcon_u72_limbs qminus = falcon_gaussian0_sda_q_limbs();
    falcon_u72_limbs r;
    uint32_t borrow;
    qminus.v0--;
    r.v0 = (qminus.v0 - x.v0) & 0xFFFFFFu;
    borrow = qminus.v0 < x.v0;
    r.v1 = (qminus.v1 - x.v1 - borrow) & 0xFFFFFFu;
    borrow = qminus.v1 < x.v1 + borrow;
    r.v2 = qminus.v2 - x.v2 - borrow;
    return r;
}

int falcon_original_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out) {
    if (!out) return -1;
    *out = map_original(falcon_u72_limbs_from_sdat(x));
    return *out <= FALCON_BASE_SUPPORT_MAX ? 0 : -2;
}

uint32_t falcon_sda_gaussian0_cumulative_lookup_for_test(sdat_u72 x) {
    return online_lookup_u72(x, (const sdat_u72 *)sda_table_falcon_base.thresholds,
                             sda_table_falcon_base.threshold_count);
}

int falcon_sda_gaussian0_sample_from_u72(sdat_u72 x, uint32_t *out,
                                         int *accepted) {
    falcon_u72_limbs candidate;
    if (!out || !accepted) return -1;
    candidate = falcon_u72_limbs_from_sdat(x);
    if (!falcon_u72_limbs_lt(candidate, falcon_gaussian0_sda_q_limbs())) {
        *accepted = 0;
        return 0;
    }
    *accepted = 1;
    *out = map_sda(sda_reverse_coordinate(candidate));
    return *out <= FALCON_BASE_SUPPORT_MAX ? 0 : -2;
}

int falcon_original_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx,
                                     uint32_t *out, sdat_stats *stats) {
    sdat_u72 x;
    int rc = draw_u72(randombytes, ctx, &x, stats, 0);
    return rc ? rc : falcon_original_gaussian0_sample_from_u72(x, out);
}

size_t falcon_original_gaussian0_sample_n(sdat_randombytes_fn randombytes,
                                          void *ctx, uint32_t *out, size_t n,
                                          sdat_stats *stats) {
    size_t i;
    if ((!out || !randombytes) && n) return 0;
    if (stats) *stats = (sdat_stats){0};
    for (i = 0; i < n; i++) {
        uint8_t b[FALCON_BASE_RANDOM_BYTES];
        uint32_t y;
        if (randombytes(ctx, b, sizeof b)) return i;
        y = map_original(falcon_u72_limbs_from_le9(b));
        if (y > FALCON_BASE_SUPPORT_MAX) return i;
        out[i] = y;
        if (stats) {
            stats->attempts++;
            stats->random_bytes += sizeof b;
            stats->random_bits += 72;
        }
    }
    return i;
}

int falcon_sda_gaussian0_sample(sdat_randombytes_fn randombytes, void *ctx,
                                uint32_t *out, sdat_stats *stats) {
    if (!out) return -1;
    for (;;) {
        sdat_u72 x;
        int accepted = 0;
        int rc = draw_u72(randombytes, ctx, &x, 0, 0);
        if (rc) return rc;
        rc = falcon_sda_gaussian0_sample_from_u72(x, out, &accepted);
        if (stats) {
            stats->attempts++;
            stats->random_bytes += FALCON_BASE_RANDOM_BYTES;
            stats->random_bits += 72;
            if (!accepted) stats->rejections++;
        }
        if (rc || accepted) return rc;
    }
}

size_t falcon_sda_gaussian0_sample_n(sdat_randombytes_fn randombytes, void *ctx,
                                     uint32_t *out, size_t n,
                                     sdat_stats *stats) {
    const falcon_u72_limbs q = falcon_gaussian0_sda_q_limbs();
    size_t i;
    if ((!out || !randombytes) && n) return 0;
    if (stats) *stats = (sdat_stats){0};
    for (i = 0; i < n; i++) {
        falcon_u72_limbs x;
        do {
            uint8_t b[FALCON_BASE_RANDOM_BYTES];
            if (randombytes(ctx, b, sizeof b)) return i;
            x = falcon_u72_limbs_from_le9(b);
            if (stats) {
                stats->attempts++;
                stats->random_bytes += sizeof b;
                stats->random_bits += 72;
                if (!falcon_u72_limbs_lt(x, q)) stats->rejections++;
            }
        } while (!falcon_u72_limbs_lt(x, q));
        out[i] = map_sda(sda_reverse_coordinate(x));
        if (out[i] > FALCON_BASE_SUPPORT_MAX) return i;
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

static uint64_t stage_mark(falcon_stage_timing *timing) {
    return timing && timing->read ? timing->read(timing->context) : 0;
}

static int workspace_ok(const falcon_stage_workspace *workspace, size_t block) {
    return workspace && workspace->candidates && block && workspace->capacity >= block;
}

static int stage_map(uint32_t *out, const falcon_u72_limbs *candidates, size_t n,
                     const falcon_u72_limbs *table) {
    for (size_t i = 0; i < n; i++) {
        out[i] = falcon_gaussian0_reverse_tail_lookup(candidates[i], table);
        if (out[i] > FALCON_BASE_SUPPORT_MAX) return -1;
    }
    return 0;
}

int falcon_original_block_staged_sample_n(
    uint32_t *out, size_t n, const uint8_t *raw, size_t raw_len, size_t block,
    falcon_stage_workspace *workspace, sdat_stats *stats,
    falcon_stage_timing *timing) {
    size_t done = 0;
    uint64_t outer0;
    if ((!out && n) || (!raw && n) || !workspace_ok(workspace, block)
            || raw_len < n * FALCON_BASE_RANDOM_BYTES) return -1;
    if (stats) *stats = (sdat_stats){0};
    if (timing) timing->input_cycles = timing->mapping_cycles = timing->outer_cycles = 0;
    outer0 = stage_mark(timing);
    while (done < n) {
        size_t take = n - done < block ? n - done : block;
        uint64_t t0 = stage_mark(timing);
        for (size_t i = 0; i < take; i++)
            workspace->candidates[i] = falcon_u72_limbs_from_le9(
                raw + (done + i) * FALCON_BASE_RANDOM_BYTES);
        uint64_t t1 = stage_mark(timing);
        if (stage_map(out + done, workspace->candidates, take,
                      falcon_gaussian0_original_reverse_tail_table())) return -2;
        uint64_t t2 = stage_mark(timing);
        if (timing && timing->read) {
            timing->input_cycles += t1 - t0;
            timing->mapping_cycles += t2 - t1;
        }
        done += take;
    }
    if (timing && timing->read) timing->outer_cycles = stage_mark(timing) - outer0;
    if (stats) {
        stats->attempts = n;
        stats->random_bytes = n * FALCON_BASE_RANDOM_BYTES;
        stats->random_bits = n * 72;
    }
    return 0;
}

int falcon_sda_block_staged_sample_n(
    uint32_t *out, size_t n, const uint8_t *raw, size_t raw_len, size_t block,
    falcon_stage_workspace *workspace, sdat_stats *stats,
    falcon_stage_timing *timing) {
    const falcon_u72_limbs q = falcon_gaussian0_sda_q_limbs();
    size_t done = 0, pos = 0;
    uint64_t outer0;
    if ((!out && n) || (!raw && n) || !workspace_ok(workspace, block)) return -1;
    if (stats) *stats = (sdat_stats){0};
    if (timing) timing->input_cycles = timing->mapping_cycles = timing->outer_cycles = 0;
    outer0 = stage_mark(timing);
    while (done < n) {
        size_t take = n - done < block ? n - done : block;
        size_t accepted = 0;
        uint64_t t0 = stage_mark(timing);
        while (accepted < take) {
            falcon_u72_limbs x;
            if (pos + FALCON_BASE_RANDOM_BYTES > raw_len) return -2;
            x = falcon_u72_limbs_from_le9(raw + pos);
            pos += FALCON_BASE_RANDOM_BYTES;
            if (falcon_u72_limbs_lt(x, q))
                workspace->candidates[accepted++] = sda_reverse_coordinate(x);
        }
        uint64_t t1 = stage_mark(timing);
        if (stage_map(out + done, workspace->candidates, take,
                      falcon_gaussian0_sda_reverse_tail_table())) return -3;
        uint64_t t2 = stage_mark(timing);
        if (timing && timing->read) {
            timing->input_cycles += t1 - t0;
            timing->mapping_cycles += t2 - t1;
        }
        done += take;
    }
    if (timing && timing->read) timing->outer_cycles = stage_mark(timing) - outer0;
    if (stats) {
        stats->attempts = pos / FALCON_BASE_RANDOM_BYTES;
        stats->rejections = stats->attempts - n;
        stats->random_bytes = pos;
        stats->random_bits = stats->attempts * 72;
    }
    return 0;
}
