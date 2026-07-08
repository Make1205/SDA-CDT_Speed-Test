#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sda_generated_tables.h"
#include "classical_cdt_generated_tables.h"
#include "original_baseline_tables.h"
#include "sda_sampler.h"
#include "sda_cycles.h"

static int cmpd(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static double bench_one(const sda_table *t, const char *kind) {
    const int reps = 11, samples = 200000, warm = 1000;
    double vals[11];
    uint64_t overhead = sda_cycles_measure_overhead();
    volatile long sink = 0;
    sda_rng_stats total = {0};
    for (int r = 0; r < reps; r++) {
        sda_bench_rng rng;
        sda_bench_rng_init(&rng, 1234 + r);
        sda_rng_stats st = {0};
        int x = 0;
        for (int i = 0; i < warm; i++) sda_sample_symmetric(t, sda_bench_random_bytes, &rng, &st, 1, &x);
        uint64_t a = sda_cycles_start();
        for (int i = 0; i < samples; i++) {
            sda_sample_symmetric(t, sda_bench_random_bytes, &rng, &st, 1, &x);
            sink += x;
        }
        uint64_t b = sda_cycles_stop();
        uint64_t cyc = (b > a) ? b - a : 0;
        vals[r] = (double)(cyc > overhead ? cyc - overhead : cyc) / (double)samples;
        total.accepted_samples += st.accepted_samples;
        total.attempts += st.attempts;
        total.rejected_candidates += st.rejected_candidates;
        total.raw_random_bits += st.raw_random_bits;
        total.accepted_random_bits += st.accepted_random_bits;
    }
    qsort(vals, reps, sizeof(double), cmpd);
    double med = vals[reps / 2], p10 = vals[1], p90 = vals[9], min = vals[0], max = vals[10];
    double dev[11];
    for (int i = 0; i < reps; i++) dev[i] = fabs(vals[i] - med);
    qsort(dev, reps, sizeof(double), cmpd);
    char q[64];
    sda_print_u128(t->denominator, q, sizeof q);
    double attempts = (double)total.attempts / (double)total.accepted_samples;
    double accbits = (double)t->denominator_bits;
    double rawbits = (double)((unsigned long long)total.raw_random_bits) / (double)total.accepted_samples;
    FILE *csv = fopen("generated/sda_cycle_benchmark.csv", "a");
    if (csv) {
        fprintf(csv, "%s,%s,%s,%d,%zu,%zu,%zu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%llu\n",
                t->parameter_set, kind, q, t->denominator_bits, t->table_length, t->ideal_packed_bits,
                sda_table_native_bytes(t), med, dev[reps / 2], p10, p90, min, max, attempts,
                (double)total.rejected_candidates / (double)total.accepted_samples, accbits, rawbits,
                (unsigned long long)overhead);
        fclose(csv);
    }
    FILE *raw = fopen("generated/sda_cycle_raw_samples.csv", "a");
    if (raw) {
        for (int rr = 0; rr < reps; rr++) fprintf(raw, "%s,%s,%d,%.6f\n", t->parameter_set, kind, rr, vals[rr]);
        fclose(raw);
    }
    printf("%s,%s,q=%s,q_bits=%d,entries=%zu,fixed_packed_bits=%zu,native_cumulative_bytes=%zu,median_cycles=%.2f,MAD=%.2f,p10=%.2f,p90=%.2f,min=%.2f,max=%.2f,attempts=%.4f,accepted_bits=%.2f,measured_raw_bits=%.2f,sink=%ld\n",
           t->parameter_set, kind, q, t->denominator_bits, t->table_length, t->ideal_packed_bits,
           sda_table_native_bytes(t), med, dev[reps / 2], p10, p90, min, max, attempts, accbits, rawbits, (long)sink);
    return med;
}

static const sda_table *find_classical(const char *parameter_set) {
    for (size_t j = 0; j < classical_cdt_generated_tables_count; j++)
        if (!strcmp(classical_cdt_generated_tables[j].parameter_set, parameter_set)) return &classical_cdt_generated_tables[j];
    return NULL;
}

static const sda_table *find_original(const char *parameter_set) {
    for (size_t j = 0; j < original_baseline_tables_count; j++)
        if (!strcmp(original_baseline_tables[j]->parameter_set, parameter_set)) return original_baseline_tables[j];
    return NULL;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    sda_cycles_pin_to_cpu(0);
    printf("cycle_benchmark,timer=%s,serialization=CPUID/RDTSC...RDTSCP/CPUID,cpu=%s,overhead=%llu\n",
           sda_cycles_supported() ? "x86-tsc" : "clock-fallback", sda_cycles_cpu_model(),
           (unsigned long long)sda_cycles_measure_overhead());
    FILE *csv = fopen("generated/sda_cycle_benchmark.csv", "w");
    if (csv) {
        fprintf(csv, "parameter_set,sampler_type,q,q_bits,table_entries,fixed_packed_bits,native_cumulative_bytes,median_cycles_per_sample,mad_cycles_per_sample,p10_cycles_per_sample,p90_cycles_per_sample,min_cycles_per_sample,max_cycles_per_sample,attempts_per_sample,rejected_candidates_per_sample,accepted_bits_per_sample,measured_raw_bits_per_sample,timer_overhead_cycles\n");
        fclose(csv);
    }
    FILE *raw = fopen("generated/sda_cycle_raw_samples.csv", "w");
    if (raw) {
        fprintf(raw, "parameter_set,sampler_type,repetition,cycles_per_sample\n");
        fclose(raw);
    }
    for (size_t i = 0; i < sda_generated_tables_count; i++) {
        const sda_table *s = &sda_generated_tables[i];
        const sda_table *o = find_original(s->parameter_set);
        const sda_table *c = find_classical(s->parameter_set);
        double ocy = 0.0, ccy = 0.0, scy = 0.0;
        if (o) ocy = bench_one(o, "original-frodo-cdt");
        if (c) ccy = bench_one(c, "regenerated-classical-cdt");
        scy = bench_one(s, "selected-sda-cdt");
        if (o && ocy > 0.0) printf("%s,speedup_vs_original_percent=%.2f\n", s->parameter_set, ((ocy - scy) / ocy) * 100.0);
        else printf("%s,speedup_vs_original_unavailable\n", s->parameter_set);
        if (c && ccy > 0.0) printf("%s,speedup_vs_regenerated_cdt_percent=%.2f\n", s->parameter_set, ((ccy - scy) / ccy) * 100.0);
        else printf("%s,speedup_vs_regenerated_cdt_unavailable\n", s->parameter_set);
    }
    return 0;
}
