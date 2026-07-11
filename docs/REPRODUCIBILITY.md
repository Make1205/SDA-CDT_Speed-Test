# Reproducibility

Regenerated research files are ignored under `offline/generated/`. To reproduce a small Frodo candidate and verification run:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

For online benchmark smoke testing:

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target benchmark_sdat_online -j
ONLINE_BENCH_SAMPLES=10000 ONLINE_BENCH_REPETITIONS=3 ./build-online/benchmark_sdat_online
```

Long Falcon BKZ and full formal benchmark runs are intentionally not part of the smoke path.


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
