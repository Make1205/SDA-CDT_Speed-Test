# C Benchmarks

The maintained C17 benchmark executables are `benchmark_frodo`,
`benchmark_falcon_base_sampler`, and `benchmark_falcon_breakdown`. Frodo deliberately has
one canonical executable and one runner:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
```

## Canonical Frodo methodology

The paper-authoritative rows are only `full-sampler-core / original-reference` and
`full-sampler-core / sda-word-reference`. Both use the reference backend. The latter is
the production word-oriented `branchless-map-before-accept` variant. They are emitted
once by the canonical driver's `full` scope into `frodo_full_sampler_raw.csv`.

`frodo_microbench_raw.csv` contains only:

* `standalone-materialized-input-frontend`, which materializes equal candidate/sign arrays;
* `standalone-cdt-mapping`, which maps inputs prepared before timing; and
* `diagnostic-full-sampler`, which compares the branchless diagnostic baseline with
  `sda-diagnostic-accept-before-map`.

The frontend and mapping measurements are independent microbenchmarks, not additive
stages of the fused full sampler. Neither they, the diagnostic, nor packed-bit rows are
sources for the main reference speed result. Optional `packed-bit-full-sampler` rows go
to the full CSV when `FRODO_BENCH_SCOPES=full,micro,packed`; they describe a different
randomness/physical-consumption interface and are not the word-oriented main result.

All raw words and packed buffers are generated before timing from a deterministic seed
of parameter set, explicit process index, repetition, and stream ID. PID is metadata
only. Timed regions exclude SHAKE, PRG, system RNG, fill, allocation, checksum, CSV, and
statistics. Full timed calls are no-stats; an untimed identical-input replay supplies
metadata and must have the same checksum.

On x86 the shared timer uses a compiler barrier and `lfence; rdtsc` at entry, then
`rdtscp; lfence` and a compiler barrier at exit. Other platforms use `CLOCK_MONOTONIC`.
Pair order reverses by repetition parity and CSV records `first`/`second`; parameter order
rotates, and the three microbenchmark groups use a fixed permutation of repetition and
parameter ID. Warm-ups never produce rows.

The runner defaults to five sequential processes, 31 repetitions, five warm-ups,
1,048,576 equal-size outputs, and scopes `full,micro`. `FRODO_BENCH_PROCESS_INDEX` is
passed explicitly. `FRODO_BENCH_CPU` enables same-core taskset pinning. The runner keeps
one header, does not aggregate cycles, and validates field counts, row counts, status,
warm-up absence, and unique `(parameter_set,timing_scope,implementation,process_index,repetition)`
keys.

## Compilation policy and AVX2 status

The canonical driver and portable reference sampler library use `-O3 -fno-lto` and have
compiler vectorization disabled (`-fno-tree-vectorize -fno-tree-slp-vectorize` for GCC,
the corresponding flags for Clang). Interprocedural optimization is disabled. Compiler
identity, version, policy flags, and configured Git commit are included in every row.
The separately compiled AVX2 library remains available. The optional `avx2` scope emits only the genuine packed-bit AVX2 path; it never emits a word-oriented AVX2 row: `frodo_sda_word_sample_n_avx2` currently
delegates to the scalar word implementation, so there is no genuine SDA word-oriented
AVX2 implementation and no misleading `sda-word-avx2` result.

Falcon benchmarks remain separate and are unaffected by the Frodo driver consolidation.
