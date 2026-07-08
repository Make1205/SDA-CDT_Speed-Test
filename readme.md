# SDA-CDT C17 offline/online workflow

This repository is a C17 implementation of an offline and online SDA-CDT workflow. Production tables are generated from distribution parameters, support, precision and solver settings; historical paper tables live only under `test/fixtures/` for regression comparison and are not included by production generation code.

## Dependencies and build

Required: GMP and MPFR. Optional but required for Falcon `flint-lll`: FLINT.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDA_ENABLE_FLINT=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Production generation

```sh
./build/generate_sdat --all --reproducible
./build/generate_sdat --config config/frodo640.conf --solver exact-denominator
./build/generate_sdat --config config/falcon.conf --solver flint-lll
./build/verify_sdat --all
```

The production path is:

```text
config -> MPFR conditioned-support target distribution -> denominator search -> normalized integer masses -> cumulative thresholds -> tail/sd_support/sd_infinite/RD metrics -> generated C/CSV/report files
```

`--all` does not use target-q fixtures. `target-q` is reserved for historical/debug work and must not overwrite production output unless a future explicit nonproduction flag is added.

## Distribution parameterization

Config files give standard deviation `sigma`. The code converts to the paper parameter

```text
gaussian_s = sigma * sqrt(2*pi)
```

for `rho_{c,s}(x)=exp(-pi(x-c)^2/s^2)`. Reports include both `sigma` in config and `gaussian_s` in the generation report.

The implemented target policy is `target_distribution=conditioned_support`. `D_infinity` is the infinite discrete Gaussian, `S` is the configured finite support, and generated integer tables approximate `D_S = D_infinity | S`. Tail mass is reported separately and is not silently dropped.

## Solvers

* Frodo uses `exact-denominator`: every `q=1..2^k-1` is scanned. For each fixed `q`, normalized integer masses are obtained by deterministic fixed-q min-max rounding with the constraint `sum p_i=q`.
* Falcon uses `flint-lll`: FLINT LLL is actually linked when `-DSDA_ENABLE_FLINT=ON`; the resulting path is marked heuristic, not exact SVP. The implementation uses FLINT as a deterministic candidate-generation hook and then normalizes/validates with MPFR. It must not be described as BKZ or exact SVP.
* Historical `target-q`/paper values are kept only as reference fixtures and comparison data.

The Frodo selection rule is: smallest denominator bit length, then smallest `q`, then smallest final `sd_infinite`, then smallest scaled error, then lexicographically smallest probability vector.

## Generated files

`generate_sdat` writes:

* `generated/sda_generated_tables.h`
* `generated/sda_tables.csv`
* `generated/sda_metrics.csv`
* `generated/sda_generation_report.txt`
* `generated/sda_candidate_report.csv`

The generated header records `SDA_GENERATED_SOURCE_IS_FIXTURE 0`. Reports include generation mode, GMP/MPFR/FLINT status, solver labels, exact/heuristic flags, tail, sd_support, sd_infinite, RD, packed bits and native cumulative bytes.

## Memory metrics

Fixed packed bits are defined as `N * ceil(log2(q))` for cumulative-table payload. Native cumulative bytes are `N * sizeof(sda_u128)`. These are separate from full C table object/native metadata bytes.

The historical Falcon denominator `4696835740265763827900` has bit length 72, so 19 fixed-width cumulative entries would use `19*72 = 1368` fixed packed bits.

## Benchmarks

```sh
./build/benchmark_offline --all
./build/benchmark_sampling --all
```

Benchmarks use a benchmark-only RNG callback, monotonic clock, warm-up, repeated runs and a result sink. Do not use the benchmark RNG in production cryptographic code. Current `benchmark_sampling` reports generated SDA-CDT table speed and memory metrics; the framework is ready for generated classical CDT side-by-side reporting, but this patch does not claim old C++ benchmark numbers as CDT speedups.

## Reference fixtures

Historical Frodo masses and the old non-monotone Falcon cumulative table are under `test/fixtures/`. `test_nonmonotone_fixture` proves the old Falcon non-monotone data is rejected by table validation. Production code does not include these fixture translation units.

## Expected differences from paper tables

Generated denominators and masses may differ from historical manuscript tables. That is expected: production generation does not hard-code manuscript denominators, probability masses or cumulative thresholds.

## Exact SVP status and cycle timing update

Rényi divergence is evaluated in the main direction `D_SDA || D_target`: zero SDA mass on a positive target coordinate contributes zero rather than infinity; positive SDA mass on zero target mass remains infinite.

Cycle benchmarks use `sda_cycles_start()`/`sda_cycles_stop()` with CPUID/RDTSC and RDTSCP/CPUID serialization on x86. The benchmark pins to logical CPU 0 when the OS permits it, reports timestamp overhead, and writes `generated/sda_cycle_benchmark.csv`. Classical CDT rows are not emitted until the generator writes real classical CDT tables; speedup is reported as unavailable rather than using a placeholder.

## Current solver metadata and unresolved exact-SVP work

The generated Frodo production tables in this repository are currently produced by `exact-denominator-search`: an exhaustive scan of `q = 1..2^k-1` with exact fixed-`q` normalization. They are **not** certified results of the SDA lattice `exact-linf-svp` solver. Generated metadata therefore uses:

- `solver = exact-denominator-search`
- `denominator_search_exact = true`
- `fixed_q_normalization_exact = true`
- `exact_linf_svp = false`
- `global_svp_certified = false`
- `lattice_solver_used = false`

The `exact-linf-svp` production path is intentionally unresolved in this snapshot; commands that require full exact certification for all parameter sets fail when Falcon cannot be certified. `generate_sdat --all --require-exact` returns exit code `3` and prints `Falcon exact generation unresolved`. Use `generate_sdat --all-available --require-exact --reproducible` to write only the currently available Frodo tables; that mode returns exit code `2` for partial success.

Metrics now distinguish `tail_mass`, `sd_support = Delta(D_S,D_SDA)`, and `sd_infinite = Delta(D_infinity,D_SDA)`. Rényi divergence fields are informational for Frodo unless `renyi_hard_constraint=true` is provided by a configuration and satisfied by the generator.

The sampling benchmark does not emit placeholder speedups. Until a generated classical CDT table is emitted alongside every SDA-CDT table, benchmark output marks `classical-cdt` speedup as unavailable.
