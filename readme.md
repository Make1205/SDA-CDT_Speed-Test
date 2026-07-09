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

## Specialized SDA exact-linf-SVP path

This snapshot adds a specialized SDA `exact-linf-svp` path for Frodo-sized supports. For the SDA basis `B = [[C I, -C alpha], [0, 1]]`, fixed `q` separates across coordinates: each raw SVP coefficient `p_i` is the nearest integer to `q alpha_i`, and the candidate norm is `max(q, C max_i ||q alpha_i||_Z)`. The solver therefore enumerates positive `q` below the current best certified upper bound and records `raw_svp_q`, `raw_svp_norm`, `raw_svp_pmf_valid`, `exact_linf_svp`, `global_svp_certified`, and `enumerated_q_count` in the generated report.

The raw SVP vector is kept separate from the online probability table. The generated SDA-CDT table uses the certified SVP denominator and then applies `fixed-q-minmax` normalization when needed; this is recorded with `pmf_is_fixed_q_normalized=true` and must not be described as the raw SVP vector. Falcon exact certification remains unresolved and is not included in the production header.

`generate_sdat --all-available --require-certified-linf-svp --reproducible` writes the currently certified Frodo tables and returns partial-success exit code 2 because Falcon is unresolved. `generate_classical_cdt --all --reproducible` regenerates the matching classical CDT baseline from the same conditioned-support target distribution with denominator `2^k`.

Note: Frodo generated headers still use the existing `sda_u128` storage path in this snapshot, so native byte counts are conservative for Frodo. The fixed packed bit counts remain `N * ceil(log2(q))`; true minimal native-width arrays are still pending.

## Current certified SDA generation status

This repository distinguishes the compact-table search objectives explicitly:

* `exact-denominator-search` is an exhaustive search over normalized PMFs and is **not** a lattice SVP certificate.
* `exact-linf-svp` is the SDA-specialized lattice objective `F(q)=max(q, C max_i ||q alpha_i||_Z)`.  For fixed `q`, nearest integers are selected coordinate-wise; global certification enumerates all `1 <= q < R_best` using MPFR directed-rounding intervals.
* Raw SVP vectors and normalized probability masses are reported separately.  If fixed-`q` min-max normalization is used, the normalized PMF is not described as the raw shortest lattice vector.

The generated Frodo SDA tables are interval-certified with `search_space_exhausted`, `nearest_integer_certified`, `norm_comparisons_certified`, `interval_certified`, and `global_svp_certified` metadata.  Falcon remains `exact_generation_unresolved` and is not emitted in the production header.

## Native integer widths and benchmarks

Generated online cumulative arrays use the smallest C integer type that stores the terminal cumulative value `q`: `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, or `sda_u128`.  Current Frodo SDA and classical CDT tables use `uint16_t` arrays.  `fixed_packed_bits` is the fixed-width payload `N * denominator_bits`, while `native_cumulative_bytes` is the online cumulative array size only.

`benchmark_sampling --all --cycles` uses serialized TSC timing (`CPUID/RDTSC ... RDTSCP/CPUID`), a benchmark-only RNG, identical scan logic for classical CDT and SDA-CDT, and writes both summary and raw repetition CSV files under `generated/`.

## Rényi and verification caveats

The main Rényi direction is `R_a(D_SDA || D_target)`.  Frodo RD values are informational in the current configs because no hard composed-security parameters are configured; finite RD is not reported as a security pass.  `verify_sdat --all` recomputes target distributions, metrics, native table widths, and reruns the SDA-specialized certificate path for production SDA tables.

## Historical baseline-dominating application selection (research/reference only)

This workflow no longer requires the final SDA-CDT application table to have negligible SD or Rényi divergence against the ideal Gaussian.  Instead, the production search imports the original Frodo sampler tables, recomputes their `sd_support`, `sd_infinite`, and main-direction Rényi values from integer masses, and accepts an SDA candidate only when the corresponding metrics are not worse than the original baseline.  The original baseline tables are emitted separately in `generated/original_baseline_tables.h`; the regenerated classical CDT in `generated/classical_cdt_generated_tables.h` is a separate comparison point and is not used as the original baseline.

The exact SDA-specialized `linf` SVP denominator remains recorded as theoretical metadata (`exact_svp_q`, raw coefficients, norm interval, and certificate fields).  Historically, the application denominator was selected by `baseline-dominating-power2-close`: first require a valid normalized PMF and certified baseline dominance, then minimize RNG draw bits `ceil(log2(q))`, and within the same draw width maximize `q / 2^ceil(log2(q))` to minimize rejection.  If `application_q != exact_svp_q`, reports set `final_q_from_exact_svp=false`; the application PMF must not be described as a shortest lattice vector.

For bounded uniform sampling the reports distinguish:

* `draw_bits = ceil(log2(q))`, the number of random bits drawn per rejection attempt;
* `threshold_bits = ceil(log2(q+1))`, the bits needed to store the terminal cumulative threshold `q`;
* `power_of_two_ceiling`, `absolute_power2_gap`, `relative_power2_gap`, `acceptance_ratio`, `expected_attempts`, and `expected_raw_bits`.

Candidate and frontier files (`generated/sda_application_candidates.csv` and `generated/sda_pareto_frontier.csv`) record the selected baseline-dominating application candidates.  For Frodo the implemented search evaluates denominators by increasing draw width and descending `q`, so once lower draw widths have no feasible candidate, the first feasible denominator in the current width is the closest-to-power-of-two application optimum for that rule.

Falcon is treated conservatively: the old non-monotone Falcon fixture is a regression test for rejection, not a legal baseline.  Because this repository does not currently contain a validated original Falcon sampler table importer, Falcon baseline comparison and production application selection remain unavailable; heuristic Falcon outputs must stay outside the production header.

Cycle benchmarks now report three common-harness rows where available: `original-frodo-cdt`, `regenerated-classical-cdt`, and `selected-sda-cdt`.  Reported speedups are labeled against the original baseline and regenerated CDT separately.  Native memory counts are actual C cumulative-array bytes plus denominator storage as reported, while packed payload uses the threshold-bit model; equal `uint16_t` arrays do not imply native byte savings even when packed payload bits differ.

## Epsilon-driven exact SDA-CDT production

Production SDA-CDT generation no longer treats `q` as an application-level target.  The production path is `epsilon -> B_epsilon -> exact l_infinity SVP -> (p_epsilon,q_epsilon) -> SDAT_epsilon`: each epsilon constructs an independent SDA lattice with `C_epsilon = epsilon^{-(n+1)}`, and the denominator is the last integer coefficient of the certified shortest vector.  The specialized SDA solver enumerates `q` only as an internal exact lattice coefficient enumeration for `F_epsilon(q)=max(q,C_epsilon max_i ||q alpha_i||_Z)`; this is an optimization of the paper's generic exact l_infinity enumeration for the SDA basis, not an approximate algorithm and not an application-level arbitrary-q search.

The generic `exact-linf-enumeration` backend accepts an upper-triangular basis and records coefficients, shortest vector, norm interval, search counters, precision, and certification flags.  Small-dimension tests and provenance tests exercise the relationship between the generic algorithmic contract and the SDA-specialized path.  Falcon now has a research-only heuristic epsilon-BKZ base-SDAT path; it still lacks a validated official baseline importer and has no certified global shortest-vector proof, so Falcon data must stay research-only and out of the production header.

For each SVP-produced denominator, the raw numerator is checked as a PMF.  If it is not a valid nonnegative table summing to the SVP denominator, fixed-q normalization may adjust only the numerator; it must preserve `q`.  Final production candidates are selected only from the epsilon-SVP-generated set, with baseline dominance required, then q ascending as the primary objective and upper power-of-two gap only as a secondary tie-breaker.  The epsilon-to-q map may be piecewise constant and non-monotone; adaptive transition refinement begins from a deterministic grid, inserts midpoints when endpoint q/vector changes are observed, and marks unchanged regions as observed-stable rather than mathematically transition-free unless a stricter certificate is implemented.

Cycle benchmarks compare original Frodo CDT, regenerated classical CDT, and the new epsilon-SVP-selected SDA-CDT in the same common harness; historical direct-q SDA values are only comparison data and are not production inputs. Falcon BKZ research artifacts are generated separately under `generated/research/falcon/`; because BKZ is heuristic and the official baseline importer remains unavailable, Falcon must not be labeled exact SVP or production-ready.

## Compact Frodo feasibility and historical rejection baseline

Frodo production candidates now have an explicit rejection-performance hard constraint in addition to exact epsilon-SVP provenance, `q < 2^15`, PMF validity, and strict SD/RD improvement over the recomputed original Frodo 2^15 CDT PMF.  The historical manuscript SDA denominators (`14534`, `7442`, and `102`) are not solver inputs and are not target denominators; they define only the minimum acceptable online acceptance ratios for Frodo-640, Frodo-976, and Frodo-1344 respectively.  If no epsilon-SVP candidate satisfies all hard constraints, the production table set is intentionally empty and the candidate traces in `generated/sda_all_candidates.csv` and `generated/sda_rejected_candidates.csv` explain the rejected candidates.

The original Frodo baseline PMFs are reconstructed as denominator-32768 tables in `generated/original_baseline_tables.h` and mirrored by `src/sda_baseline.c` for metric recomputation.  Paper rounded log2 values are retained only as regression diagnostics in `generated/original_baseline_metrics.csv`; they are not used as the production comparison input.

Falcon research support includes a 72-bit bounded-uniform/base-sampler wrapper and a real 20-dimensional epsilon-BKZ base-SDAT run recorded in `generated/research/falcon/`.  The selected Falcon table remains research-only because no official Falcon base-sampler constants have been imported and BKZ does not certify global SVP optimality.

### Falcon offline epsilon-BKZ SDAT research pipeline

Falcon is handled as a research-only **base SDAT** workflow.  The full Falcon discrete-Gaussian sampler and Bernoulli-exponential outer loop are intentionally not integrated here.  The offline flow is:

`Falcon base target distribution -> alpha -> B_epsilon -> integer row embedding -> fplll BKZ -> recovered (p,q) -> fixed-q normalization (if needed) -> MPFR post-verification`.

The selected Falcon research table is therefore labelled `solver = falcon-epsilon-bkz`, `heuristic_bkz = true`, `exact_linf_svp = false`, and `global_svp_certified = false`.  BKZ only supplies heuristic candidate vectors; every candidate is rechecked against the original real SDA l_infinity objective and the Rényi requirement `R_a <= 1 + 2^-78`.  The denominator `q` is recovered as the last coefficient in the row-basis combination and is never fixed to the paper historical denominator.  Historical Falcon `q = 4696835740265763827900` is recorded only as regression context.

The generated selected table is written under `generated/research/falcon/` with unambiguous 72-bit limb encoding: `low64, high8` little-endian cumulative thresholds.  Run `tools/generate_falcon_sdat.py` to regenerate the research artifacts and `tools/verify_falcon_sdat.py` to independently re-check the selected PMF, denominator bound, cumulative thresholds, BKZ provenance, and Rényi bound.
