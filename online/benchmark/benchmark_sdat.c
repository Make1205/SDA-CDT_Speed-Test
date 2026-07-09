#include "sdat_ref.h"
#include "sdat_avx2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

typedef struct { uint64_t s; } rng;
static int rb(void *c, uint8_t *out, size_t n) {
    rng *r = (rng *)c;
    for (size_t i = 0; i < n; i++) {
        r->s = r->s * 2862933555777941757ULL + 3037000493ULL;
        out[i] = (uint8_t)(r->s >> 32);
    }
    return 0;
}
static uint64_t rd(void) { unsigned aux; _mm_lfence(); uint64_t r = __rdtscp(&aux); _mm_lfence(); return r; }
static uint64_t timer_overhead(void) { uint64_t best = UINT64_MAX; for (int i = 0; i < 1000; i++) { uint64_t a = rd(), b = rd(); if (b - a < best) best = b - a; } return best; }
static size_t envsz(const char *n, size_t d) { char *s = getenv(n); return (s && *s) ? strtoull(s, 0, 10) : d; }
static int want_kind(const char *want, const char *kind) { return !strcmp(want, "all") || !strcmp(want, kind); }
static double expected_acceptance(const sdat_table *t, int sda) {
    if (!sda) return 1.0;
    long double top = 1.0L;
    for (unsigned i = 0; i < t->random_draw_bits; i++) top *= 2.0L;
    long double q = t->denominator_u64 ? (long double)t->denominator_u64 : ((long double)t->denominator_u72.hi * 18446744073709551616.0L + (long double)t->denominator_u72.lo);
    return (double)(q / top);
}
static int parse_batches(const char *s, int *out, int maxn) {
    int n = 0; char *tmp = strdup(s ? s : "1,4,8,16,64,256,1024"); if (!tmp) return 0;
    for (char *p = strtok(tmp, ","); p && n < maxn; p = strtok(NULL, ",")) out[n++] = atoi(p);
    free(tmp); return n;
}
static uint64_t checksum32(const uint32_t *x, size_t n) { uint64_t h = 1469598103934665603ULL; for (size_t i = 0; i < n; i++) { h ^= x[i]; h *= 1099511628211ULL; } return h; }
static void fill_lookup_inputs_u8(uint8_t *x, size_t n, const sdat_table *t, int sda) { uint64_t mod = sda ? t->denominator_u64 : (1ULL << t->random_draw_bits); for (size_t i = 0; i < n; i++) x[i] = (uint8_t)(i % mod); }
static void fill_lookup_inputs_u16(uint16_t *x, size_t n, const sdat_table *t, int sda) { uint64_t mod = sda ? t->denominator_u64 : (1ULL << t->random_draw_bits); for (size_t i = 0; i < n; i++) x[i] = (uint16_t)(i % mod); }
static void fill_lookup_inputs_u72(sdat_u72 *x, size_t n) { for (size_t i = 0; i < n; i++) x[i] = (sdat_u72){(uint64_t)i, 0}; }

typedef struct { uint64_t cycles, attempts, rejections, random_bits, random_bytes, checksum; sdat_avx2_stats avx; } result;

static int bench_sampler(const sdat_table *t, int sda, int avx, size_t n, size_t batch, unsigned rep, result *res) {
    uint32_t *out = calloc(batch ? batch : 1, sizeof *out); if (!out) return -1;
    rng r = {123 + 0x9e3779b97f4a7c15ULL * (uint64_t)(rep + 1)}; sdat_stats st = {0}; online_avx2_stats_reset();
    uint64_t total = 0, h = 1469598103934665603ULL; size_t done = 0; uint64_t t0 = rd();
    while (done < n) {
        size_t m = n - done < batch ? n - done : batch;
        int rc = sda ? (avx ? sda_cdt_avx2_sample_batch(t, rb, &r, out, m, &st) : sda_cdt_ref_sample_batch(t, rb, &r, out, m, &st))
                     : (avx ? original_cdt_avx2_sample_batch(t, rb, &r, out, m, &st) : original_cdt_ref_sample_batch(t, rb, &r, out, m, &st));
        if (rc) { free(out); return rc; }
        h ^= checksum32(out, m) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        done += m; total += m;
    }
    uint64_t t1 = rd(); (void)total;
    res->cycles = t1 - t0; res->attempts = st.attempts; res->rejections = st.rejections; res->random_bits = st.random_bits; res->random_bytes = st.random_bytes; res->checksum = h; res->avx = *online_avx2_stats(); free(out); return 0;
}

static int bench_lookup(const sdat_table *t, int sda, int avx, size_t n, size_t batch, result *res) {
    uint32_t *out = calloc(batch ? batch : 1, sizeof *out); if (!out) return -1; online_avx2_stats_reset();
    uint64_t h = 1469598103934665603ULL; size_t done = 0; uint64_t t0 = rd();
    while (done < n) {
        size_t m = n - done < batch ? n - done : batch;
        if (t->value_type == SDAT_TYPE_U8) { uint8_t *x = calloc(m, sizeof *x); if (!x) return -2; fill_lookup_inputs_u8(x, m, t, sda); if (avx) sda_cdt_avx2_lookup_u8_batch(x, m, t->thresholds, t->threshold_count, out); else for (size_t i = 0; i < m; i++) out[i] = online_lookup_u8(x[i], t->thresholds, t->threshold_count); free(x); }
        else if (t->value_type == SDAT_TYPE_U16) { uint16_t *x = calloc(m, sizeof *x); if (!x) return -3; fill_lookup_inputs_u16(x, m, t, sda); if (avx) original_cdt_avx2_lookup_u16_batch(x, m, t->thresholds, t->threshold_count, out); else for (size_t i = 0; i < m; i++) out[i] = online_lookup_u16(x[i], t->thresholds, t->threshold_count); free(x); }
        else { sdat_u72 *x = calloc(m, sizeof *x); if (!x) return -4; fill_lookup_inputs_u72(x, m); if (avx) { if (!sda && !strcmp(t->table_family, "original-cdt-table")) original_cdt_avx2_lookup_u72_reverse_batch(x, m, t->thresholds, t->threshold_count, out); else sda_cdt_avx2_lookup_u72_batch(x, m, t->thresholds, t->threshold_count, out); } else { for (size_t i = 0; i < m; i++) out[i] = (!sda && !strcmp(t->table_family, "original-cdt-table")) ? online_lookup_u72_reverse_tail(x[i], t->thresholds, t->threshold_count) : online_lookup_u72(x[i], t->thresholds, t->threshold_count); } free(x); }
        h ^= checksum32(out, m) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2); done += m;
    }
    uint64_t t1 = rd(); res->cycles = t1 - t0; res->attempts = res->rejections = res->random_bits = res->random_bytes = 0; res->checksum = h; res->avx = *online_avx2_stats(); free(out); return 0;
}

static void print_header(void) {
    puts("scheme,parameter_set,sampler_family,table_family,implementation,benchmark_kind,batch_size,lane_width,repetition_index,repetitions,samples_per_repetition,total_cycles,cycles_per_sample,timer_overhead,attempts,rejections,random_bits,random_bytes,attempts_per_sample,rejections_per_sample,acceptance_ratio,expected_acceptance_ratio,random_bits_per_sample,random_bytes_per_sample,native_table_bytes,packed_bits,vector_batches,vectorized_samples,scalar_tail_samples,fallback_samples,refill_rounds,rejected_lanes,avx2_path_executed,checksum,status,reason");
}
static void print_row(const sdat_table *t, const char *sf, const char *impl, const char *kind, int batch, unsigned rep, unsigned reps, size_t n, uint64_t overhead, result r) {
    int avx = strstr(impl, "avx2") != 0; int sda = !strcmp(sf, "sda-cdt");
    printf("%s,%s,%s,%s,%s,%s,%d,%d,%u,%u,%zu,%llu,%.6f,%llu,%llu,%llu,%llu,%llu,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%zu,%zu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,available,\n",
        t->scheme, t->parameter_set, sf, t->table_family, impl, kind, batch, avx ? (t->value_type == SDAT_TYPE_U72 ? 4 : 8) : 1, rep, reps, n,
        (unsigned long long)r.cycles, (double)r.cycles / (double)n, (unsigned long long)overhead,
        (unsigned long long)r.attempts, (unsigned long long)r.rejections, (unsigned long long)r.random_bits, (unsigned long long)r.random_bytes,
        n ? (double)r.attempts / (double)n : 0.0, n ? (double)r.rejections / (double)n : 0.0,
        r.attempts ? 1.0 - ((double)r.rejections / (double)r.attempts) : 1.0, expected_acceptance(t, sda),
        n ? (double)r.random_bits / (double)n : 0.0, n ? (double)r.random_bytes / (double)n : 0.0,
        t->native_table_bytes, t->packed_bits,
        (unsigned long long)r.avx.avx2_vector_batches, (unsigned long long)r.avx.avx2_vectorized_samples,
        (unsigned long long)r.avx.scalar_tail_samples, (unsigned long long)r.avx.fallback_samples,
        (unsigned long long)r.avx.refill_rounds, (unsigned long long)r.avx.rejected_lanes,
        (unsigned long long)(r.avx.avx2_vectorized_samples > 0), (unsigned long long)r.checksum);
}
int main(void) {
    size_t n = envsz("ONLINE_BENCH_SAMPLES", 1000000), warm = envsz("ONLINE_BENCH_WARMUP", 100000);
    unsigned reps = (unsigned)envsz("ONLINE_BENCH_REPETITIONS", 21); const char *kind = getenv("ONLINE_BENCH_KIND"); if (!kind) kind = "all";
    int batches[64]; int nb = parse_batches(getenv("ONLINE_BENCH_BATCH_SIZES"), batches, 64); if (!nb) return 2;
    const sdat_table *tabs[] = {&original_cdt_table_frodo640,&sda_table_frodo640,&original_cdt_table_frodo976,&sda_table_frodo976,&original_cdt_table_frodo1344,&sda_table_frodo1344,&original_cdt_table_falcon_base,&sda_table_falcon_base};
    uint64_t overhead = timer_overhead(); print_header();
    for (size_t ti = 0; ti < sizeof tabs / sizeof *tabs; ti++) {
        const sdat_table *t = tabs[ti]; int is_sda = !strcmp(t->table_family, "sda-table"); const char *sf = is_sda ? "sda-cdt" : "original-cdt";
        for (int bi = 0; bi < nb; bi++) for (unsigned rep = 0; rep < reps; rep++) {
            int batch = batches[bi]; result r = {0};
            if (warm) { result w = {0}; if (want_kind(kind, "end-to-end")) bench_sampler(t, is_sda, 0, warm, (size_t)batch, rep, &w); }
            if (want_kind(kind, "lookup-only")) { if (bench_lookup(t, is_sda, 0, n, (size_t)batch, &r)) return 3; print_row(t, sf, is_sda ? "sda-table-ref" : "original-cdt-ref", "lookup-only", batch, rep, reps, n, overhead, r); if (sdat_avx2_cpu_supported()) { if (bench_lookup(t, is_sda, 1, n, (size_t)batch, &r)) return 4; print_row(t, sf, is_sda ? "sda-table-avx2" : "original-cdt-avx2", "lookup-only", batch, rep, reps, n, overhead, r); } }
            if (want_kind(kind, "end-to-end")) { if (bench_sampler(t, is_sda, 0, n, (size_t)batch, rep, &r)) return 5; print_row(t, sf, is_sda ? "sda-cdt-ref" : "original-cdt-ref", "end-to-end", batch, rep, reps, n, overhead, r); if (sdat_avx2_cpu_supported()) { if (bench_sampler(t, is_sda, 1, n, (size_t)batch, rep, &r)) return 6; print_row(t, sf, is_sda ? "sda-cdt-avx2" : "original-cdt-avx2", "end-to-end", batch, rep, reps, n, overhead, r); } }
        }
    }
    return 0;
}
