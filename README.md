# SDA-CDT Speed Test

This repository separates SDA-CDT work into an **online phase** and an **offline phase**.

* `online/` contains the fixed-width C sampler, final reviewed table payloads/manifests, online KAT/tests, and online benchmarks. It intentionally does not link GMP/MPFR or read `offline/generated/`.
* `offline/` contains SDA modeling, exact Frodo `l_infinity` SVP/certificate code, Falcon heuristic epsilon-BKZ research code, table generation, verification, offline tests, configs, and reproducible scripts.
* `offline/generated/` is the ignored workspace for regenerated CSV, logs, candidate tables, checkpoints, basis files, and benchmark outputs. Historical generated files from the pre-cleanup tree are kept conservatively under `offline/generated/legacy/` in the working tree but are not tracked.

## Layout

```text
.
├── online/                 # runtime sampler, final tables, online tests/benchmarks
│   ├── common/             # shared online table and bounded-uniform support
│   ├── frodo/              # scalar/reference online implementation
│   ├── falcon/             # AVX2 online implementation and Falcon base table support
│   ├── tables/             # committed final table artifacts and manifests
│   ├── tests/
│   └── benchmarks/
├── offline/                # research, generation, exact/heuristic solvers, verification
│   ├── common/             # C library used by offline tools/tests
│   ├── scripts/            # generators, verifiers, exporters, summarizers
│   ├── configs/            # Frodo/Falcon generation configs
│   ├── tests/
│   ├── benchmarks/
│   ├── results/verified/   # small committed verification summaries
│   └── generated/          # ignored regenerated outputs
├── docs/
└── third_party/
```

See `docs/REPOSITORY_LAYOUT.md` for the detailed mapping.

## Build and test only the online sampler

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target test_sdat_online benchmark_sdat_online -j
ctest --test-dir build-online -R test_sdat_online --output-on-failure
```

The online targets use only C17 and standard system libraries. They do not require GMP, MPFR, FLINT, fplll, or files under `offline/generated/`.

## Run online benchmarks

Frodo and Falcon online lookup/end-to-end rows are emitted by the same binary:

```sh
ONLINE_BENCH_SAMPLES=10000 ONLINE_BENCH_REPETITIONS=3 ./build-online/benchmark_sdat_online
```

For the scripted benchmark harness:

```sh
online/benchmarks/run_online_benchmarks.sh --quick
```

The script writes ignored output under `offline/generated/online/formal/`.

## Minimal offline generation and verification

Offline tools require GMP and MPFR; FLINT is optional unless explicitly enabling Falcon FLINT/LLL experiments.

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

Generation writes candidate and report files into `offline/generated/`. Verification reads the generated candidate header from `offline/generated/sda_generated_tables.h` when present, otherwise the conservative legacy placeholder in `offline/generated/legacy/` keeps offline tests buildable.

## Offline to online table boundary

The intended flow is:

```text
offline solver/search
  -> offline/generated candidate artifacts
  -> offline verification
  -> export reviewed table + manifest
  -> online/tables/<scheme>/<parameter-set>/ or online/common/sdat_tables.c
```

Committed online-required artifacts are:

* `online/common/sdat_tables.c` and `online/common/sdat_tables.h` (runtime table definitions and format).
* `online/tables/frodo/` (reviewed Frodo online table manifests, hashes, and exported headers).
* `online/tables/falcon/` (reviewed Falcon source/provenance table artifacts).

Regenerable candidate files, logs, basis files, checkpoints, and benchmark outputs belong in `offline/generated/` and are ignored by Git.

## Exact Frodo path vs Falcon heuristic path

Frodo offline production/certification code contains an exact SDA-specialized `l_infinity` SVP path and exact-denominator smoke paths. Falcon research artifacts use an epsilon-BKZ/`l_2` heuristic candidate-generation flow and are explicitly marked heuristic; they must not be described as exact `l_infinity` SVP certificates.

Additional details are in `docs/OFFLINE_PHASE.md`, `docs/TABLE_FORMAT.md`, and `docs/REPRODUCIBILITY.md`.
