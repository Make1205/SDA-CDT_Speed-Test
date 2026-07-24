# Reproducibility

## Correctness

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

If GMP and MPFR are installed, offline targets and tests are enabled by the same CMake project. `verify_sdat --all` intentionally fails in a clean checkout when no production tables exist in `offline/generated/`.

## Benchmarks

The maintained targets are `benchmark_frodo_sample_n`, `benchmark_frodo_breakdown`, `benchmark_falcon_base_sampler`, and `benchmark_falcon_breakdown`. Run their shell wrappers:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
benchmark/scripts/run_falcon_benchmarks.sh
```

Outputs are raw CSV under `build/benchmark-results/`; they are transient and ignored. No Python summarizer or generated summary is part of this repository.
