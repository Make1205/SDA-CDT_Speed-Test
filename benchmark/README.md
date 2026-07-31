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

## Implementation-specific local runs

The canonical C executable remains `benchmark_frodo`. Two small shell wrappers select a
single implementation without copying any benchmark logic:

```sh
benchmark/scripts/run_frodo_original.sh  # frodo_original_raw.csv
benchmark/scripts/run_frodo_sda.sh       # frodo_sda_raw.csv
```

Both accept the same `FRODO_BENCH_RESULTS_DIR`, `FRODO_BENCH_PROCESSES`,
`FRODO_BENCH_REPETITIONS`, `FRODO_BENCH_WARMUP`, `FRODO_BENCH_SAMPLE_COUNT`,
`FRODO_BENCH_MODE`, `FRODO_BENCH_SCOPES`, `FRODO_BENCH_CPU`, and `BUILD` variables.
They use the same executable, build, parameter rotation, deterministic word stream,
timer, no-stats calls, and replay checks. Rows can therefore be joined offline on
parameter set, process index, repetition, input seed, and input stream ID. Separate runs
avoid sharing momentary state inside one process, but should still be made on the same
machine and pinned CPU with identical builds and nearby system conditions.

The Original wrapper selects only Original full/frontend/mapping rows. The SDA wrapper
selects only SDA full/frontend/mapping and both SDA diagnostic rows. Their validators
reject the other implementation, warm-up rows, duplicate keys, non-`ok` status, malformed
schemas, or a missing authoritative row. These wrappers do not create another C harness.

## SDA scalar frontend audit

Release assembly showed that table/parameter dispatch occurs once before each specialized
frontend loop; q, mask, and sign shifts are immediates, and no division, modulo, function
call, stats, or checksum is present in a timed hot loop. The primary cost was the
unpredictable acceptance branch plus two accepted-output stores. Frodo-1344 rejects about
20.3% of attempts (roughly 1.255 attempts/output), making this branch particularly costly;
there was no additional Frodo-1344-only dynamic dispatch.

The specialized loops now share inline candidate/sign/accept primitives with the fused
word samplers and use branchless compacting stores: each attempt writes the current output
slot and advances candidate/sign destinations by the public acceptance bit. Rejected
writes are overwritten and are not observable. Candidate, sign, and acceptance are each
computed once. The fused production path remains branchless map-before-accept and does not
materialize intermediate arrays. Its generated hot-loop assembly is unchanged apart from
using the shared inline primitives, so medium-run differences there should be treated as
measurement variation rather than a claimed algorithmic speedup.
