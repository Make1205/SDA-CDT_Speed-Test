# SDA-CDT Speed Test

This repository is split by responsibility:

* `offline/`: table generation, exact-SVP/SDA solving, PMF/CDF/threshold export, certificates, verification tools, and offline correctness tests.
* `online/`: production sampler/runtime code, reviewed Frodo/Falcon table mappings, scalar/AVX2 implementations, and online correctness tests.
* `benchmark/`: performance-only benchmark sources, benchmark scripts, and benchmark result documentation.
* `docs/`: phase notes, table-format notes, and reproducibility instructions.

Frodo production tables are frozen in this workflow:

| Parameter set | SDA q |
| --- | ---: |
| Frodo-640 | 14534 |
| Frodo-976 | 7442 |
| Frodo-1344 | 102 |

Future epsilon-driven Frodo table research should run outside the default production workflow and must not overwrite frozen production artifacts.

## Build and test

Online-only correctness build:

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

Offline generation/verification build:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-offline -j
./build-offline/verify_sdat --all
ctest --test-dir build-offline --output-on-failure
```

Benchmark build, enabled explicitly:

```sh
cmake -S . -B build-benchmark -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-benchmark --target benchmark_frodo_sample_n benchmark_sdat_online -j
```

Benchmark scripts live under `benchmark/scripts/` and default to writing transient output under `build/benchmark-results/`.

## Frodo sampler scope

The Frodo `sample_n` code is a one-dimensional sampler benchmark harness only. It does not implement FrodoKEM KeyGen, Encaps, or Decaps. Original Frodo keeps the official 16-bit word semantics; SDA supports packed-bit and word-oriented unbiased rejection profiles. Timing and metrics benchmark passes remain separated so performance measurements do not include per-sample instrumentation.

## Production table boundary

Production runtime tables are the definitions in `online/common/sdat_tables.c` and the reviewed manifest under `online/tables/frodo/`. Regenerated candidate files, research traces, solver logs, and benchmark CSV outputs are not production artifacts and should not be committed unless deliberately promoted through the documented verification path.
