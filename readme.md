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
config -> MPFR conditioned-support target distribution -> denominator search -> normalized integer masses -> cumulative thresholds -> SD/RD/tail metrics -> generated C/CSV/report files
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

The Frodo selection rule is: smallest denominator bit length, then smallest `q`, then smallest final SD, then smallest scaled error, then lexicographically smallest probability vector.

## Generated files

`generate_sdat` writes:

* `generated/sda_generated_tables.h`
* `generated/sda_tables.csv`
* `generated/sda_metrics.csv`
* `generated/sda_generation_report.txt`
* `generated/sda_candidate_report.csv`

The generated header records `SDA_GENERATED_SOURCE_IS_FIXTURE 0`. Reports include generation mode, GMP/MPFR/FLINT status, solver labels, exact/heuristic flags, tail, SD, RD, packed bits and native cumulative bytes.

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
