# C Benchmarks

Four C17 benchmarks are maintained:

- `benchmark_frodo_sample_n`
- `benchmark_frodo_breakdown`
- `benchmark_falcon_base_sampler`
- `benchmark_falcon_breakdown`

Run them through:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
benchmark/scripts/run_falcon_benchmarks.sh
```

Each runner configures a Release build with `SDA_BUILD_BENCHMARKS=ON`, builds only its two benchmark targets, runs them, and writes raw CSV under `build/benchmark-results/`. Environment variables accepted by the benchmark executables can be exported before invoking a runner. Statistical summarization is intentionally outside this lightweight repository.

The benchmarks use the frozen online tables and do not generate or modify table parameters. Frodo measures the one-dimensional sampler core; Falcon measures its base half-Gaussian sampler, not full signing.

## Frodo fairness scopes

The Frodo breakdown CSV fixes three independent scopes. **standalone materialized input
frontend** reads precomputed 16-bit words and writes candidate and sign arrays (SDA also
performs rejection). **standalone-cdt-mapping** receives candidates/signs prepared before
timing and measures only lookup, sign application, and equal-sized output writes.
**full-sampler-core** reads precomputed words and includes candidate/sign extraction, SDA
rejection, CDT mapping, compaction, and output writes. These microbenchmarks are not an
additive decomposition: in particular, `full sampler != frontend + mapping` because the
full path is fused.

No Frodo timed region contains SHAKE, PRG, system RNG, `fill16`, `fill8`, allocation,
input construction, checksum, CSV output, or statistics collection. Checksums and a
stats replay of identical inputs occur after timing; the replay must match the timed
no-stats output. On x86 the timer is `lfence; rdtsc` at entry and `rdtscp; lfence` at
exit, with compiler memory barriers. Other targets use `CLOCK_MONOTONIC` nanoseconds.
Tables are intentionally hot. Repetition parity gives deterministic paired,
counterbalanced order (even: Original then SDA; odd: SDA then Original), while parameter
order rotates; warm-ups are omitted from CSV.

`run_frodo_benchmarks.sh` runs sequential processes (default
`FRODO_BENCH_PROCESSES=5`), each with 31 repetitions, and combines their unmodified rows
under one header. Set `FRODO_BENCH_CPU` to pin each sequential process with `taskset`.
`FRODO_BENCH_NATIVE_BATCH=0` selects equal-size; setting it to `1` selects each parameter's
native batch. Raw cycles are never aggregated by the runner.

The full-core diagnostic reports both the production branchless map-before-accept path
(which maps rejected candidates before conditional destination advancement) and a
reference accept-before-map path (which skips lookup for rejection). They must emit the
same sequence; performance rows quantify the otherwise extra rejected-candidate lookups
without replacing production behavior.
