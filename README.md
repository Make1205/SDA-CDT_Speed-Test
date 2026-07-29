# SDA-CDT Speed Test

A C17 repository for SDA-CDT table generation, validation, online sampling, and focused benchmarks.

- `offline/` contains the GMP/MPFR-based C generator, exact l-infinity SVP solver, validator, configurations, and unit tests.
- `online/` contains dependency-free Frodo and Falcon runtime samplers, frozen tables, and tests.
- `benchmark/` contains the four maintained C benchmarks and simple shell runners.
- `docs/` describes the offline/online boundary and table conventions.

The checked-in online tables are frozen: Frodo-640 uses q=14534, Frodo-976 q=7442, Frodo-1344 q=102, and the existing Falcon SDA table is unchanged. Candidate generation writes only to the ignored `offline/generated/` workspace.

## Build and test

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

Online targets do not require GMP or MPFR. When both dependencies are installed, the same configuration also enables the C offline tools `generate_sdat`, `verify_sdat`, and `export_tables`, plus their unit tests.

Benchmarks are opt-in:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
benchmark/scripts/run_falcon_benchmarks.sh
```

The runners build C targets and write raw CSV only under `build/benchmark-results/`.
