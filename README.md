# SDA-CDT Speed Test

This repository is split by responsibility:

* `offline/`: table generation, exact-SVP/SDA solving, PMF/CDF/threshold export, certificates, verification tools, and offline correctness tests.
* `online/`: production sampler/runtime code, reviewed Frodo/Falcon table mappings, reference/AVX2 backend implementations, and online correctness tests.
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

## Falcon base sampler

The online Falcon addition is limited to the portable C half-Gaussian base sampler over support `{0,...,18}` with center 0 and sigma0=1.8205. It exposes Original and SDA base-sampler APIs for correctness tests and benchmarks only; it does not implement samplerZ, BerExp, FFT sampling, signing, verification, keygen, or any full Falcon-512/Falcon-1024 API. Benchmark code for the base sampler lives in `benchmark/falcon/`.

## Reporting policy

Paper-primary speed results should compare portable/reference Original against portable/reference SDA-CDT at the same benchmark scope. AVX2 implementations and cross-ISA comparisons are retained for correctness checks and future optimization work, but they are not the current paper-primary speedup claim.

## Frodo sampler architecture

Frodo uses one sampler framework.  Frodo-640, Frodo-976, and Frodo-1344 differ only by parameter data (q, candidate bit width, sign position, thresholds, threshold counts, support, and native batch metadata) plus backend-internal compile-time specializations selected through the shared dispatcher.  Original CDT and SDA-CDT are sampler kinds; `reference` and `avx2` are backends.  The reference backend is portable C compiled with the selected optimization flags, so compiler auto-vectorization may occur and it must not be described as a strict scalar-instruction implementation.  AVX2 is a separate backend implemented in AVX2-specific objects and is future-work for paper-primary speed claims.

Paper-primary Frodo speed results compare `original-reference` with `sda-word-reference` at the same parameter set and benchmark mode.  Packed SDA rows describe the packed-bit frontend and randomness accounting separately.  AVX2 rows are valid diagnostics/future-work rows and must only be compared against AVX2 rows.
