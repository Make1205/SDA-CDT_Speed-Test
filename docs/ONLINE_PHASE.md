# Online phase

The online phase executes table lookup and bounded uniform sampling from reviewed SDA-CDT/original-CDT table data. Build it with:

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target test_sdat_online benchmark_sdat_online -j
ctest --test-dir build-online -R test_sdat_online --output-on-failure
```

The online phase uses `online/common`, `online/frodo`, `online/falcon`, and committed metadata in `online/tables`. It does not depend on `offline/generated`.


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
