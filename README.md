# SDA-CDT Speed Test

This repository separates SDA-CDT work into an **online phase** and an **offline phase**.

* `online/` contains the fixed-width C sampler, final reviewed table payloads/manifests, online KAT/tests, and online benchmarks. It intentionally does not link GMP/MPFR or read `offline/generated/`.
* `offline/` contains SDA modeling, exact Frodo `l_infinity` SVP/certificate code, Falcon heuristic epsilon-BKZ research code, table generation, verification, offline tests, configs, and reproducible scripts.
* `offline/generated/` is the ignored workspace for regenerated CSV, logs, candidate tables, checkpoints, basis files, and benchmark outputs. Historical generated files from the pre-cleanup tree are kept conservatively under `offline/generated/legacy/` in the working tree but are not tracked.

## Layout

```text
.
├── online/                 # runtime sampler, final tables, online tests/benchmarks
│   ├── common/             # shared online table and bounded-uniform support
│   ├── frodo/              # scalar/reference online implementation
│   ├── falcon/             # AVX2 online implementation and Falcon base table support
│   ├── tables/             # committed final table artifacts and manifests
│   ├── tests/
│   └── benchmarks/
├── offline/                # research, generation, exact/heuristic solvers, verification
│   ├── common/             # C library used by offline tools/tests
│   ├── scripts/            # generators, verifiers, exporters, summarizers
│   ├── configs/            # Frodo/Falcon generation configs
│   ├── tests/
│   ├── benchmarks/
│   ├── results/verified/   # small committed verification summaries
│   └── generated/          # ignored regenerated outputs
├── docs/
└── third_party/
```

See `docs/REPOSITORY_LAYOUT.md` for the detailed mapping.

## Build and test only the online sampler

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target test_sdat_online benchmark_sdat_online -j
ctest --test-dir build-online -R test_sdat_online --output-on-failure
```

The online targets use only C17 and standard system libraries. They do not require GMP, MPFR, FLINT, fplll, or files under `offline/generated/`.

## Run online benchmarks

Frodo and Falcon online lookup/end-to-end rows are emitted by the same binary:

```sh
ONLINE_BENCH_SAMPLES=10000 ONLINE_BENCH_REPETITIONS=3 ./build-online/benchmark_sdat_online
```

For the scripted benchmark harness:

```sh
online/benchmarks/run_online_benchmarks.sh --quick
```

The script writes ignored output under `offline/generated/online/formal/`.

## Minimal offline generation and verification

Offline tools require GMP and MPFR; FLINT is optional unless explicitly enabling Falcon FLINT/LLL experiments.

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

Generation writes candidate and report files into `offline/generated/`. Verification reads the generated candidate header from `offline/generated/sda_generated_tables.h` when present, otherwise the conservative legacy placeholder in `offline/generated/legacy/` keeps offline tests buildable.

## Offline to online table boundary

The intended flow is:

```text
offline solver/search
  -> offline/generated candidate artifacts
  -> offline verification
  -> export reviewed table + manifest
  -> online/tables/<scheme>/<parameter-set>/ or online/common/sdat_tables.c
```

Committed online-required artifacts are:

* `online/common/sdat_tables.c` and `online/common/sdat_tables.h` (runtime table definitions and format).
* `online/tables/frodo/` (reviewed Frodo online table manifests, hashes, and exported headers).
* `online/tables/falcon/` (reviewed Falcon source/provenance table artifacts).

Regenerable candidate files, logs, basis files, checkpoints, and benchmark outputs belong in `offline/generated/` and are ignored by Git.

## Exact Frodo path vs Falcon heuristic path

Frodo offline production/certification code contains an exact SDA-specialized `l_infinity` SVP path and exact-denominator smoke paths. Falcon research artifacts use an epsilon-BKZ/`l_2` heuristic candidate-generation flow and are explicitly marked heuristic; they must not be described as exact `l_infinity` SVP certificates.

Additional details are in `docs/OFFLINE_PHASE.md`, `docs/TABLE_FORMAT.md`, and `docs/REPRODUCIBILITY.md`.


## Frodo scheme-faithful sample_n sampler

This tree now includes a Frodo-only one-dimensional sampler API that is intentionally separate from KEM KeyGen/Encaps/Decaps. `frodo_original_sample_n(uint16_t *s, size_t n, const sdat_table *table)` matches the official FrodoKEM `frodo_sample_n` core contract: callers prefill `s` with little-endian PRG words; each word is split as `prnd = word >> 1` and `sign = word & 1`; a fixed-length branchless CDT scan maps `prnd` to a magnitude; and the array is overwritten with `((-sign) ^ magnitude) + sign` in modulo-`uint16_t` representation.

`frodo_sda_sample_n` consumes a prefilled byte buffer through `sdat_bitreader`. The bitstream is LSB-first within each byte. It takes `b = ceil(log2(q))` candidate bits, rejects candidates `x >= q` without consuming a sign bit, and only after acceptance consumes one sign bit. This reports candidate bits, sign bits, rejections/attempts, and source bytes separately; it never uses `% q` or a biased fallback.

The scheme-faithful benchmark target is `benchmark_frodo_sample_n`. It keeps the legacy generic-harness benchmark (`benchmark_sdat_online`) and labels the new measurements as `sample_n-core`; random buffer allocation/filling and CSV formatting are outside the timed core. The CSV exposes accepted bits, expected/observed raw bits through attempts/rejections, and source bytes so 13/14 logical bits are not confused with 16-bit word reads. Original Frodo uses 15 magnitude bits plus one sign bit per sample. Frodo-640 SDA may consume more expected raw bits than the 16-bit classical baseline.

Frodo table-size reporting distinguishes `ideal_variable_packed_bits = sum bit_length(threshold_i)`, `fixed_width_packed_bits = threshold_count * ceil(log2(q))`, and `native_storage_bytes = threshold_count * sizeof(native_entry_type)`. Current ideal packed bits are Original/SDA: Frodo-640 192/165, Frodo-976 162/127, Frodo-1344 104/33. Scalar and AVX2 paths cover only the Frodo integer sampler; these results do not claim full FrodoKEM KeyGen, Encaps, or Decaps speedups.

Reproduce online-only Frodo checks with:

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target test_frodo_sample_n benchmark_frodo_sample_n -j
ctest --test-dir build-online -R frodo_sample_n --output-on-failure
./build-online/benchmark_frodo_sample_n
```


### Frodo sampler optimization profiles and benchmark passes

`benchmark_frodo_sample_n` now separates `measurement_pass=timing` from `measurement_pass=metrics`. Timing rows call SDA samplers with `stats = NULL`; attempts, rejections, raw-bit counters, source-byte counters, and refill counters are collected only by the untimed metrics pass over identical input buffers, and the benchmark checks that timing and metrics checksums match.

Two SDA profiles are reported. `packed-bit` is the bit-efficient profile: it consumes an LSB-first 7/13/14-bit packed reservoir and preserves leftover bits to approach theoretical raw-bit use. `word-oriented` is the cycle-efficient upper-bound profile: candidates come from prefilled 16-bit words using rejection, and an accepted candidate then consumes a separate sign word bit. Both profiles are unbiased, do not use modulo reduction, and target the same SDA rational distribution, but their randomness metrics are not interchangeable.

The Frodo hot path now has specialized extractors for 1, 7, 13, and 14 bits and specialized per-parameter samplers for Frodo-640, Frodo-976, and Frodo-1344, so the sample loop avoids per-sample table-type dispatch, runtime q/bits reloads, and stats instrumentation in timing mode. Component rows include `bitreader-only`, `candidate-extraction-only`, `uniform-bounded-only`, `sign-extraction-only`, `lookup-only`, `sign-application-only`, and `full-sample-n-core`.

Table-size reporting distinguishes `full_exported_table_bits` (the paper/exported cumulative table width, including terminal/export conventions) from `online_stored_threshold_bits` (thresholds actually stored online after terminal-threshold omission). The documented 192/165, 162/127, and 104/33 values are the ideal variable packed full/exported-table figures; online storage also reports fixed-width packed bits and native storage bytes. These sampler benchmarks do not include PRG generation or any FrodoKEM KeyGen/Encaps/Decaps work. Current smoke results should be read as sampler-only evidence; if SDA does not beat Original, the bottleneck is reported as extraction/rejection/sign handling rather than hidden.


### Word-oriented sign merge and stable benchmarking

The word-oriented SDA profile now takes both fields from one accepted 16-bit word: the low `b` bits form the candidate and bit `b` forms the sign. Because the input word is uniform, bit `b` is independent of the low `b` bits; rejection depends only on the low `b` bits, so conditioned on acceptance the sign remains uniform. Rejected words produce no output and no separate sign consumption. The logical randomness is still `b` candidate bits plus one sign bit per attempt/accepted sample as reported by metrics, while physical source bytes are `2 * attempts` rather than the previous `2 * attempts + 2`.

Stable Frodo benchmarking should be run through `online/benchmarks/run_frodo_stable_benchmark.sh`. The script records CPU model, kernel, governors, Turbo status when exposed by sysfs, the git commit, and whether `taskset` CPU affinity is available; it pins to `FRODO_BENCH_CORE` when possible but does not modify governor or Turbo settings. The benchmark CSV keeps timing and metrics rows separate.

Packed-bit remains a single-stream profile in the default benchmark. A multistream packed SIMD design would use independent lane bitstreams and would change bitstream layout while preserving the distribution; it needs PRG stream derivation/domain separation before production use. Current component data shows packed AVX2 is still limited by serial extraction/rejection and batch overhead, so no default multistream sampler is installed in this round.

Table-size columns now distinguish `full_exported_threshold_bits`, `actual_c_array_threshold_bits`, `minimal_required_threshold_bits`, and `native_storage_bytes`; these threshold-bit counts exclude support metadata, type tags, and table counts.
