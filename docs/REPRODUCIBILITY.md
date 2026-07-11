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
