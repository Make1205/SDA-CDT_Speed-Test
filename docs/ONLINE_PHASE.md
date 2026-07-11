# Online Phase

The online phase owns production runtime sampling only: table mapping, bounded-uniform/rejection sampling, scalar and AVX2 sampler implementations, and correctness tests. Performance benchmark sources and scripts live under `benchmark/`.

Frodo production SDA q values are frozen at 14534/7442/102 for Frodo-640/976/1344. Original Frodo CDT thresholds remain the official FrodoKEM-compatible CDFs.

Build online correctness tests without benchmarks:

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

The Frodo `sample_n` code remains a one-dimensional sampler harness only; it does not implement KeyGen, Encaps, or Decaps. Timing/metrics-separated performance runs are built explicitly from `benchmark/` with `SDA_BUILD_BENCHMARKS=ON`.
