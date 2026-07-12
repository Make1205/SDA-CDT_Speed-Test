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

## Reporting policy

Use `paper-primary` for portable/reference Original-vs-SDA rows at the same scope (`mapping_only` or `end_to_end`). Treat AVX2 rows as `future-work`: valid for internal diagnostics, but not as the current paper-primary comparison. Do not mix word-oriented speed with packed-bit physical-source accounting without labelling the trade-off.

## Frodo sample_n benchmark labels

`benchmark_frodo_sample_n` reports explicit dimensions: `sampler_kind`, `backend`, `frontend`, `mode`, and `implementation`.  The canonical implementation labels are:

| Deprecated label | Canonical label |
| --- | --- |
| `official-original-scalar` | `original-reference` |
| `official-original-avx2` | `original-avx2` |
| `sda-packed-scalar` | `sda-packed-reference` |
| `sda-packed-avx2` | `sda-packed-avx2` |
| `sda-word-scalar` | `sda-word-reference` |
| `sda-word-avx2` | `sda-word-avx2` |

The benchmark defaults to equal-size throughput (`FRODO_BENCH_SAMPLE_COUNT=16384`).  Set `FRODO_BENCH_NATIVE_BATCH=1` to use Frodo-native batch sizes (5120, 7808, 10752).  `FRODO_BENCH_REPETITIONS`, `FRODO_BENCH_WARMUP`, and `FRODO_BENCH_ORDER_SEED` control repetitions, warmup, and deterministic parameter/order rotation.  Paper-primary rows are reference-vs-reference only; AVX2 rows are future-work diagnostics.
