# Benchmarks

This directory contains performance-only code and scripts. Production table
generation/certification remains under `offline/`, and runtime samplers remain
under `online/`.

Frodo production tables are frozen for this workflow:

- Frodo-640 SDA q = 14534
- Frodo-976 SDA q = 7442
- Frodo-1344 SDA q = 102

Build benchmark targets explicitly with `-DSDA_BUILD_BENCHMARKS=ON`, for
example:

```sh
cmake -S . -B build-benchmark -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-benchmark --target benchmark_frodo_sample_n benchmark_sdat_online -j
```

Run scripts from any directory; outputs default to `build/benchmark-results/`
or an environment-provided output directory. Do not commit transient CSV output.

## Falcon base-sampler benchmark

`benchmark_falcon_base_sampler` compares `falcon_original_portable` and `falcon_sda_portable` in `mapping_only` and `end_to_end` modes. Both variants use the same deterministic random-byte backend, report attempts/rejections/source bits/source bytes, and keep raw CSV output outside tracked source paths by default.
