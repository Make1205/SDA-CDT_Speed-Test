# Reproducibility

## Offline verification

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-offline -j
./build-offline/verify_sdat --all
ctest --test-dir build-offline --output-on-failure
```

## Online correctness

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

## Benchmarks

Benchmarks are opt-in and live under `benchmark/`:

```sh
cmake -S . -B build-benchmark -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-benchmark --target benchmark_frodo_sample_n benchmark_sdat_online -j
benchmark/scripts/run_frodo_benchmarks.sh
```

Benchmark scripts default to `build/benchmark-results/` and should not write tracked CSV files. Frodo production tables are frozen at q=14534/7442/102; research searches for future tables are outside the default production workflow.

## Falcon base-sampler benchmark

```sh
cmake -S . -B build-benchmark -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-benchmark --target benchmark_falcon_base_sampler -j
./build-benchmark/benchmark_falcon_base_sampler
```

This benchmark covers only the portable half-Gaussian base sampler and does not include full Falcon signing.

## Paper-primary benchmark interpretation

For the current paper draft, report portable/reference Original vs portable/reference SDA-CDT first. AVX2 benchmark rows are reproducible diagnostics and future-work data, not the main speedup claim. Keep `mapping_only` and `end_to_end` summaries separate.

## Frodo benchmark modes and labels

For Frodo sample_n results, use the canonical labels emitted by `benchmark_frodo_sample_n`: `original-reference`, `sda-word-reference`, `sda-packed-reference`, `original-avx2`, `sda-word-avx2`, and `sda-packed-avx2`.  The word `reference` denotes portable C compiled with the selected optimization flags; compiler auto-vectorization may occur.  Do not call these rows strict scalar unless using a separately documented strict-scalar-audit build with vectorization disabled.

Equal-size throughput:

```sh
FRODO_BENCH_SAMPLE_COUNT=16384 FRODO_BENCH_REPETITIONS=31 ./build-benchmark/benchmark_frodo_sample_n
```

Scheme-native batch sizes:

```sh
FRODO_BENCH_NATIVE_BATCH=1 FRODO_BENCH_REPETITIONS=31 ./build-benchmark/benchmark_frodo_sample_n
```

The current paper-primary Frodo comparison is `original-reference` vs `sda-word-reference` for the same parameter set and mode.  AVX2 is reproducible future-work data and must not be mixed with reference backend rows.
